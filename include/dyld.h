#ifndef DYLD_H
#define DYLD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <stdint.h>

// check if a library is loaded
bool SPUDyldIsLibraryLoaded(const char* libraryPath);

// to load a dynamic library
void* SPUDyldLoadLibrary(const char* libraryPath);

// get a symbol from a loaded library
void* SPUDyldGetSymbol(void* handle, const char* symbolName);

// unload a dynamic library
void SPUDyldUnloadLibrary(void* handle);

// get the last error message
const char* SPUDyldGetLastError(void);

// get the path of the currently executing image
const char* SPUDyldGetExecutablePath(void);

// check if code signing is enforced
bool SPUDyldIsCodeSigningEnforced(void);

#ifdef __cplusplus
}
#endif

#endif