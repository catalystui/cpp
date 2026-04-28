#include "CATCRLIB.hpp"
#include "CATMMLIB.h"

#include <mach/mach.h>

namespace catalyst {
namespace memory {

    typedef struct DARWIN_MEMHEAD {
        NUINT total_size;
        NUINT requested_size;
    } DARWIN_MEMHEAD;

} /* namespace memory */
} /* namespace catalyst */

using namespace catalyst;
using namespace catalyst::memory;

void catmmAlloc(void** memory, NUINT size, RESULT* result) {
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

    // Check total size for overflow
    NUINT total_size = (NUINT) sizeof(DARWIN_MEMHEAD) + size;
    if (total_size < size) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }

    // Allocate address
    vm_address_t address = 0;
    if (vm_allocate(mach_task_self(), &address, (vm_size_t) total_size, VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 0, 0);
        return;
    }

    // Initialize memory and return
    DARWIN_MEMHEAD* header = (DARWIN_MEMHEAD*) address;
    header->total_size = total_size;
    header->requested_size = size;
    *memory = (void*) (((BYTE*) address) + sizeof(DARWIN_MEMHEAD));
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

void catmmRealloc(void** memory, NUINT size, RESULT* result) {
    // Validate, initialize parameters
    if (memory == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (*memory == 0) {
        catmmAlloc(memory, size, result);
        return;
    }
    if (size == 0) {
        RESULT free_result;
        catmmFree(*memory, &free_result);
        if (statusCodeIsSuccess(free_result.status)) {
            *memory = 0;
        }
        if (result != 0) *result = free_result;
        return;
    }

    // Get old block information
    BYTE* old_bytes = (BYTE*) *memory;
    memory::DARWIN_MEMHEAD* old_header = (memory::DARWIN_MEMHEAD*) (old_bytes - sizeof(memory::DARWIN_MEMHEAD));

    // Allocate the new block
    void* new_memory = 0;
    RESULT alloc_result;
    catmmAlloc(&new_memory, size, &alloc_result);
    if (!statusCodeIsSuccess(alloc_result.status)) {
        if (result != 0) *result = alloc_result;
        return;
    }

    // Copy the data
    BYTE* new_bytes = (BYTE*) new_memory;
    NUINT copy_size = old_header->requested_size < size ? old_header->requested_size : size;
    for (NUINT i = 0; i < copy_size; i++) {
        new_bytes[i] = old_bytes[i];
    }

    // Free the old block
    RESULT free_result;
    catmmFree(*memory, &free_result);
    if (!statusCodeIsSuccess(free_result.status)) {
        // Restore original state if we failed to free the old block
        catmmFree(new_memory, 0);
        if (result != 0) *result = free_result;
        return;
    }

    // Return the new block
    *memory = new_memory;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

void catmmFree(void* memory, RESULT* result) {
    // Validate, initialize parameters
    if (memory == 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS_NOOP, 0, 0, 0);
        return;
    }

    // Get header then deallocate address
    BYTE* bytes = (BYTE*) memory;
    DARWIN_MEMHEAD* header = (DARWIN_MEMHEAD*) (bytes - sizeof(DARWIN_MEMHEAD));
    kern_return_t kern_result = vm_deallocate(mach_task_self(), (vm_address_t) header, (vm_size_t) header->total_size);
    if (kern_result != KERN_SUCCESS) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}
