#include "../include/spulibc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

struct SPULibrary {
    char* name;
    char** sources;
    char** languages;
    size_t sourceCount;
    void** objects;
    size_t* objectSizes;
    size_t objectCount;
};

SPUError SPULibraryCreate(const char* name, SPULibraryRef* outLibrary) {
    if (!name || !outLibrary) return SPU_ERROR_INVALID_ARGUMENT;
    
    struct SPULibrary* lib = (struct SPULibrary*)calloc(1, sizeof(struct SPULibrary));
    if (!lib) return SPU_ERROR_MEMORY;
    
    lib->name = strdup(name);
    if (!lib->name) {
        free(lib);
        return SPU_ERROR_MEMORY;
    }
    
    *outLibrary = lib;
    return SPU_SUCCESS;
}

SPUError SPULibraryAddSource(SPULibraryRef library, const char* sourceCode, const char* language) {
    if (!library || !sourceCode || !language) return SPU_ERROR_INVALID_ARGUMENT;
    
    char** newSources = realloc(library->sources, (library->sourceCount + 1) * sizeof(char*));
    char** newLanguages = realloc(library->languages, (library->sourceCount + 1) * sizeof(char*));
    
    if (!newSources || !newLanguages) return SPU_ERROR_MEMORY;
    
    library->sources = newSources;
    library->languages = newLanguages;
    
    library->sources[library->sourceCount] = strdup(sourceCode);
    library->languages[library->sourceCount] = strdup(language);
    
    if (!library->sources[library->sourceCount] || !library->languages[library->sourceCount]) {
        return SPU_ERROR_MEMORY;
    }
    
    library->sourceCount++;
    return SPU_SUCCESS;
}

SPUError SPULibraryAddObject(SPULibraryRef library, const void* objectData, size_t size) {
    if (!library || !objectData || size == 0) return SPU_ERROR_INVALID_ARGUMENT;
    
    void** newObjects = realloc(library->objects, (library->objectCount + 1) * sizeof(void*));
    size_t* newSizes = realloc(library->objectSizes, (library->objectCount + 1) * sizeof(size_t));
    
    if (!newObjects || !newSizes) return SPU_ERROR_MEMORY;
    
    library->objects = newObjects;
    library->objectSizes = newSizes;
    
    library->objects[library->objectCount] = malloc(size);
    if (!library->objects[library->objectCount]) return SPU_ERROR_MEMORY;
    
    memcpy(library->objects[library->objectCount], objectData, size);
    library->objectSizes[library->objectCount] = size;
    
    library->objectCount++;
    return SPU_SUCCESS;
}

SPUError SPULibraryBuild(SPULibraryRef library, const char* outputPath) {
    if (!library || !outputPath) return SPU_ERROR_INVALID_ARGUMENT;
    
    char tempDir[1024];
    snprintf(tempDir, sizeof(tempDir), "/tmp/spulibc_XXXXXX");
    if (!mkdtemp(tempDir)) {
        printf("Failed to create temporary directory\n");
        return SPU_ERROR_IO;
    }
    
    char** objFiles = (char**)calloc(library->sourceCount, sizeof(char*));
    if (!objFiles) {
        printf("Failed to allocate memory for object files\n");
        return SPU_ERROR_MEMORY;
    }
    
    SPUError error = SPU_SUCCESS;
    size_t compiledCount = 0;
    
    for (size_t i = 0; i < library->sourceCount; i++) {
        char srcPath[1024];
        char objPath[1024];
        char srcPathWithExt[1024];
        char cmd[2048];
        
        snprintf(srcPath, sizeof(srcPath), "%s/src_%zu", tempDir, i);
        snprintf(objPath, sizeof(objPath), "%s/obj_%zu.o", tempDir, i);
        
        if (strcmp(library->languages[i], "c") == 0) {
            snprintf(srcPathWithExt, sizeof(srcPathWithExt), "%s.c", srcPath);
            snprintf(cmd, sizeof(cmd), "clang -c -arch arm64 -isysroot $(xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=12.0 -o %s %s -I . -framework Foundation -framework UIKit -DTARGET_OS_IPHONE=1", objPath, srcPathWithExt);
        } else if (strcmp(library->languages[i], "cpp") == 0) {
            snprintf(srcPathWithExt, sizeof(srcPathWithExt), "%s.cpp", srcPath);
            snprintf(cmd, sizeof(cmd), "clang++ -c -arch arm64 -isysroot $(xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=12.0 -o %s %s -I . -framework Foundation -framework UIKit -DTARGET_OS_IPHONE=1", objPath, srcPathWithExt);
        } else if (strcmp(library->languages[i], "asm") == 0) {
            snprintf(srcPathWithExt, sizeof(srcPathWithExt), "%s.s", srcPath);
            snprintf(cmd, sizeof(cmd), "as -arch arm64 -o %s %s -force_cpusubtype_ALL", objPath, srcPathWithExt);
        } else {
            error = SPU_ERROR_INVALID_ARGUMENT;
            goto cleanup;
        }
        
        FILE* srcFile = fopen(srcPathWithExt, "w");
        if (!srcFile) {
            printf("Failed to create source file: %s\n", srcPathWithExt);
            error = SPU_ERROR_IO;
            goto cleanup;
        }
        
        if (fputs(library->sources[i], srcFile) == EOF) {
            printf("Failed to write source code to file\n");
            fclose(srcFile);
            error = SPU_ERROR_IO;
            goto cleanup;
        }
        fclose(srcFile);
        
        pid_t pid;
        char *argv[] = { "sh", "-c", cmd, NULL };
        int status;
        status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, NULL);
        if (status == 0) {
            if (waitpid(pid, &status, 0) != -1) {
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    error = SPU_ERROR_LOAD;
                    goto cleanup;
                }
            } else {
                error = SPU_ERROR_LOAD;
                goto cleanup;
            }
        } else {
            error = SPU_ERROR_LOAD;
            goto cleanup;
        }
        
        objFiles[compiledCount++] = strdup(objPath);
    }
    
    char* objList = (char*)malloc(4096);
    if (!objList) {
        error = SPU_ERROR_MEMORY;
        goto cleanup;
    }
    objList[0] = '\0';
    
    for (size_t i = 0; i < compiledCount; i++) {
        strcat(objList, objFiles[i]);
        strcat(objList, " ");
    }
    
    for (size_t i = 0; i < library->objectCount; i++) {
        char objPath[1024];
        snprintf(objPath, sizeof(objPath), "%s/existing_%zu.o", tempDir, i);
        
        FILE* objFile = fopen(objPath, "wb");
        if (!objFile) {
            error = SPU_ERROR_IO;
            goto cleanup;
        }
        fwrite(library->objects[i], 1, library->objectSizes[i], objFile);
        fclose(objFile);
        
        strcat(objList, objPath);
        strcat(objList, " ");
    }
    
    char linkCmd[4096];
    snprintf(linkCmd, sizeof(linkCmd),
        "clang -shared -arch arm64 -isysroot $(xcrun --sdk iphoneos --show-sdk-path) -mios-version-min=12.0 -o %s %s -framework Foundation -framework UIKit -install_name @rpath/%s",
        outputPath, objList, strrchr(outputPath, '/') ? strrchr(outputPath, '/') + 1 : outputPath);
    
    pid_t pid;
    char *argv[] = { "sh", "-c", linkCmd, NULL };
    int status;
    status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, NULL);
    if (status == 0) {
        if (waitpid(pid, &status, 0) != -1) {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                error = SPU_ERROR_LOAD;
                goto cleanup;
            }
        } else {
            error = SPU_ERROR_LOAD;
            goto cleanup;
        }
    } else {
        error = SPU_ERROR_LOAD;
        goto cleanup;
    }
    
cleanup:
    if (objFiles) {
        for (size_t i = 0; i < compiledCount; i++) {
            if (objFiles[i]) {
                remove(objFiles[i]);
                free(objFiles[i]);
            }
        }
        free(objFiles);
    }
    free(objList);
    
    char rmCmd[1024];
    snprintf(rmCmd, sizeof(rmCmd), "rm -rf %s", tempDir);
    pid_t cleanup_pid;
    char *cleanup_argv[] = { "sh", "-c", rmCmd, NULL };
    posix_spawn(&cleanup_pid, "/bin/sh", NULL, NULL, cleanup_argv, NULL);
    waitpid(cleanup_pid, NULL, 0);
    
    return error;
}

void SPULibraryDestroy(SPULibraryRef library) {
    if (!library) return;
    
    free(library->name);
    
    for (size_t i = 0; i < library->sourceCount; i++) {
        free(library->sources[i]);
        free(library->languages[i]);
    }
    free(library->sources);
    free(library->languages);
    
    for (size_t i = 0; i < library->objectCount; i++) {
        free(library->objects[i]);
    }
    free(library->objects);
    free(library->objectSizes);
    
    free(library);
}