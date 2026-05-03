#include "CMAKECFG.h"
#if TARGET_PLATFORM_LINUX

#include "CATCRLIB.hpp"
#include "CATMMLIB.h"

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

using namespace catalyst;

extern "C" void catmmAlloc(void** memory, CATALYST_NUINT size, CATALYST_RESULT* result) {
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

    // Get system page size
    long pageSizeLong = sysconf(_SC_PAGESIZE);
    if (pageSizeLong <= 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_STATE, 0, 0, errno);
        return;
    }

    // Check against overflow, then allocate
    NUINT pageSize = (NUINT) pageSizeLong;
    NUINT headerSize = (NUINT) sizeof(CATALYST_MEMORY_HEADER);
    NUINT totalSize = headerSize + size;
    if (totalSize < size || totalSize < headerSize) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 0);
        return;
    }
    if (totalSize > ((NUINT) ~0) - (pageSize - 1)) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_BUFFER_OVERFLOW, 0, 0, 1);
        return;
    }
    NUINT allocatedSize = ((totalSize + pageSize - 1) / pageSize) * pageSize;

    // Allocate memory
    void* address = mmap(0, (size_t) allocatedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (address == MAP_FAILED) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 0, errno);
        return;
    }

    // Initialize header and return
    CATALYST_MEMORY_HEADER* header = (CATALYST_MEMORY_HEADER*) address;
    header->requested_size = size;
    header->allocated_size = allocatedSize;
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
    if (munmap((void*) header, (size_t) header->allocated_size) != 0) {
        if (result != 0) *result = RESULT(STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, errno);
        return;
    }
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}

#endif /* TARGET_PLATFORM_LINUX */

