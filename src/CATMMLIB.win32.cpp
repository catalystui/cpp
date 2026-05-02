#include "CMAKECFG.h"
#if TARGET_PLATFORM_WIN32

#include "CATCRLIB.hpp"
#include "CATMMLIB.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

using namespace catalyst;

extern "C" void catmmAlloc(void** memory, CATALYST_NUINT size, CATALYST_RESULT* result) {
    if (memory == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    *memory = 0;
    if (size == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 1);
        return;
    }

    // Check against overflow, then allocate
    NUINT totalSize = (NUINT) sizeof(CATALYST_MEMORY_HEADER) + size;
    if (totalSize < size) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }
    void* address = VirtualAlloc(0, (SIZE_T) totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (address == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 0, 0);
        return;
    }

    // Initialize header and return
    CATALYST_MEMORY_HEADER* header = (CATALYST_MEMORY_HEADER*) address;
    header->requested_size = size;
    header->allocated_size = totalSize;
    *memory = (void*) (((BYTE*) address) + sizeof(CATALYST_MEMORY_HEADER));
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

extern "C" void catmmFree(void* memory, CATALYST_RESULT* result) {
    // Validate, initialize parameters
    if (memory == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    // Get header then deallocate
    BYTE* bytes = (BYTE*) memory;
    CATALYST_MEMORY_HEADER* header = (CATALYST_MEMORY_HEADER*) (bytes - sizeof(CATALYST_MEMORY_HEADER));
    if (!VirtualFree((void*) header, 0, MEM_RELEASE)) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

#endif /* TARGET_PLATFORM_WIN32 */
