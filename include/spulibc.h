#ifndef SPULIBC_H
#define SPULIBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    SPU_SUCCESS = 0,
    SPU_ERROR_INVALID_ARGUMENT = -1,
    SPU_ERROR_MEMORY = -2,
    SPU_ERROR_IO = -3,
    SPU_ERROR_LOAD = -4
} SPUError;

typedef struct SPULibrary* SPULibraryRef;

// Library creation and destruction
SPUError SPULibraryCreate(const char* name, SPULibraryRef* outLibrary);
void SPULibraryDestroy(SPULibraryRef library);

// Source code management
SPUError SPULibraryAddSource(SPULibraryRef library, const char* sourceCode, const char* language);

// Object file management
SPUError SPULibraryAddObject(SPULibraryRef library, const void* objectData, size_t size);

// Build and output
SPUError SPULibraryBuild(SPULibraryRef library, const char* outputPath);

#ifdef __cplusplus
}
#endif

#endif