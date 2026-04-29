#include "CMAKECFG.h"
#if TARGET_PLATFORM_DARWIN

#include "CATCRLIB.hpp"
#include "CATMMLIB.h"

#import <mach/mach.h>

using namespace catalyst;

extern "C" void catmmAlloc(void** memory, NUINT size, RESULT* result) {
    // Validate, initialize parameters
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
    vm_address_t address = 0;
    if (vm_allocate(mach_task_self(), &address, (vm_size_t) totalSize, VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
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

extern "C" void catmmFree(void* memory, RESULT* result) {
    // Validate, initialize parameters
    if (memory == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    // Get header then deallocate
    BYTE* bytes = (BYTE*) memory;
    CATALYST_MEMORY_HEADER* header = (CATALYST_MEMORY_HEADER*) (bytes - sizeof(CATALYST_MEMORY_HEADER));
    kern_return_t deallocResult = vm_deallocate(mach_task_self(), (vm_address_t) header, (vm_size_t) header->allocated_size);
    if (deallocResult != KERN_SUCCESS) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

#endif /* TARGET_PLATFORM_DARWIN */
