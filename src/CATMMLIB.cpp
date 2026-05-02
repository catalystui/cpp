#include "CATCRLIB.hpp"
#include "CATMMLIB.h"

using namespace catalyst;

void catmmRealloc(void** memory, CATALYST_NUINT size, CATALYST_RESULT* result) {
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
        RESULT freeResult;
        catmmFree(*memory, &freeResult);
        if (statusCodeIsSuccess(freeResult.status)) {
            *memory = 0;
        }
        if (result != 0) *result = freeResult;
        return;
    }

    // Fetch the old block, then allocate the new one
    BYTE* oldBytes = (BYTE*) *memory;
    CATALYST_MEMORY_HEADER* oldHeader = (CATALYST_MEMORY_HEADER*) (oldBytes - sizeof(CATALYST_MEMORY_HEADER));
    void* newMemory = 0;
    RESULT allocResult;
    catmmAlloc(&newMemory, size, &allocResult);
    if (!statusCodeIsSuccess(allocResult.status)) {
        if (result != 0) *result = allocResult;
        return;
    }

    // Copy the data
    BYTE* newBytes = (BYTE*) newMemory;
    NUINT copySize = oldHeader->requested_size < size ? oldHeader->requested_size : size;
    for (NUINT i = 0; i < copySize; i++) {
        newBytes[i] = oldBytes[i];
    }

    // Free the old block and return
    RESULT freeResult;
    catmmFree(*memory, &freeResult);
    if (!statusCodeIsSuccess(freeResult.status)) {
        // Restore original state if we failed to free the old block
        catmmFree(newMemory, 0);
        if (result != 0) *result = freeResult;
        return;
    }
    *memory = newMemory;
    if (result != 0) *result = RESULT(STATUS_CODE_SUCCESS, 0, 0, 0);
}
