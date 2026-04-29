#include "CATMMLIB.hpp"

#include <stdio.h>

using namespace catalyst;

static void printResult(const char* label, const RESULT& result) {
    printf(
        "%s -> status=0x%02X context=0x%02X operation=0x%02X detail=0x%02X\n",
        label,
        (unsigned int) result.status,
        (unsigned int) result.context,
        (unsigned int) result.operation,
        (unsigned int) result.detail
    );
}

int main() {
    RESULT result;
    void* memory;
    BYTE* bytes;
    NUINT i;

    printf("CATMMTEST :: Starting allocation lifecycle test...\n");

    /* ---------------------------------------------------------------------------------------------
     * Allocate 64 bytes.
     * --------------------------------------------------------------------------------------------- */

    memory = 0;
    result = RESULT();

    printf("Allocating 64 bytes...\n");
    catalyst::alloc(&memory, 64, &result);
    printResult("alloc", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Allocation failed.\n");
        return 1;
    }

    if (memory == 0) {
        printf("Allocation returned null memory.\n");
        return 2;
    }

    printf("Memory allocated at %p\n", memory);

    /* ---------------------------------------------------------------------------------------------
     * Write and verify the first pattern.
     * --------------------------------------------------------------------------------------------- */

    bytes = (BYTE*) memory;

    printf("Writing 64-byte test pattern...\n");
    for (i = 0; i < 64; i++) {
        bytes[i] = (BYTE) i;
    }

    printf("Verifying 64-byte test pattern...\n");
    for (i = 0; i < 64; i++) {
        if (bytes[i] != (BYTE) i) {
            printf("Mismatch before realloc at index %lu: expected %lu, got %u\n", (unsigned long) i, (unsigned long) i, (unsigned int) bytes[i]);
            return 3;
        }
    }

    printf("Initial pattern verified successfully.\n");

    /* ---------------------------------------------------------------------------------------------
     * Grow allocation from 64 bytes to 128 bytes.
     * The original 64 bytes should survive.
     * --------------------------------------------------------------------------------------------- */

    printf("Reallocating 64 bytes to 128 bytes...\n");
    catalyst::realloc(&memory, 128, &result);
    printResult("realloc grow", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Realloc grow failed.\n");
        return 4;
    }

    if (memory == 0) {
        printf("Realloc grow returned null memory.\n");
        return 5;
    }

    printf("Memory after grow is at %p\n", memory);

    bytes = (BYTE*) memory;

    printf("Verifying original 64 bytes survived grow...\n");
    for (i = 0; i < 64; i++) {
        if (bytes[i] != (BYTE) i) {
            printf("Mismatch after grow at index %lu: expected %lu, got %u\n", (unsigned long) i, (unsigned long) i, (unsigned int) bytes[i]);
            return 6;
        }
    }

    printf("Writing extended 128-byte pattern...\n");
    for (i = 64; i < 128; i++) {
        bytes[i] = (BYTE) i;
    }

    printf("Verifying full 128-byte pattern...\n");
    for (i = 0; i < 128; i++) {
        if (bytes[i] != (BYTE) i) {
            printf("Mismatch after extension at index %lu: expected %lu, got %u\n", (unsigned long) i, (unsigned long) i, (unsigned int) bytes[i]);
            return 7;
        }
    }

    printf("Grow pattern verified successfully.\n");

    /* ---------------------------------------------------------------------------------------------
     * Shrink allocation from 128 bytes to 32 bytes.
     * The first 32 bytes should survive.
     * --------------------------------------------------------------------------------------------- */

    printf("Reallocating 128 bytes down to 32 bytes...\n");
    catalyst::realloc(&memory, 32, &result);
    printResult("realloc shrink", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Realloc shrink failed.\n");
        return 8;
    }

    if (memory == 0) {
        printf("Realloc shrink returned null memory.\n");
        return 9;
    }

    printf("Memory after shrink is at %p\n", memory);

    bytes = (BYTE*) memory;

    printf("Verifying first 32 bytes survived shrink...\n");
    for (i = 0; i < 32; i++) {
        if (bytes[i] != (BYTE) i) {
            printf("Mismatch after shrink at index %lu: expected %lu, got %u\n", (unsigned long) i, (unsigned long) i, (unsigned int) bytes[i]);
            return 10;
        }
    }

    printf("Shrink pattern verified successfully.\n");

    /* ---------------------------------------------------------------------------------------------
     * Realloc size 0 should behave like free and null the pointer.
     * --------------------------------------------------------------------------------------------- */

    printf("Reallocating to 0 bytes. This should free the memory...\n");
    catalyst::realloc(&memory, 0, &result);
    printResult("realloc zero", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Realloc zero/free failed.\n");
        return 11;
    }

    if (memory != 0) {
        printf("Realloc zero did not null the memory pointer.\n");
        return 12;
    }

    printf("Realloc zero/free verified successfully.\n");

    /* ---------------------------------------------------------------------------------------------
     * Realloc with null memory should behave like alloc.
     * --------------------------------------------------------------------------------------------- */

    printf("Testing realloc with null pointer. This should behave like alloc...\n");
    catalyst::realloc(&memory, 16, &result);
    printResult("realloc null", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Realloc null-as-alloc failed.\n");
        return 13;
    }

    if (memory == 0) {
        printf("Realloc null-as-alloc returned null memory.\n");
        return 14;
    }

    bytes = (BYTE*) memory;

    printf("Writing 16-byte pattern after realloc null-as-alloc...\n");
    for (i = 0; i < 16; i++) {
        bytes[i] = (BYTE) (i + 1);
    }

    printf("Verifying 16-byte pattern...\n");
    for (i = 0; i < 16; i++) {
        if (bytes[i] != (BYTE) (i + 1)) {
            printf("Mismatch after realloc null-as-alloc at index %lu: expected %lu, got %u\n", (unsigned long) i, (unsigned long) (i + 1), (unsigned int) bytes[i]);
            return 15;
        }
    }

    printf("Realloc null-as-alloc verified successfully.\n");

    /* ---------------------------------------------------------------------------------------------
     * Free final allocation.
     * --------------------------------------------------------------------------------------------- */

    printf("Freeing final allocation...\n");
    catalyst::free(memory, &result);
    printResult("free final", result);

    if (!statusCodeIsSuccess(result.status)) {
        printf("Final free failed.\n");
        return 16;
    }

    /* ---------------------------------------------------------------------------------------------
     * Invalid argument tests.
     * --------------------------------------------------------------------------------------------- */

    printf("Testing invalid realloc: memory == 0...\n");
    result = RESULT();
    catmmRealloc(0, 64, &result);
    printResult("realloc invalid memory", result);

    if (result.status != STATUS_CODE_ERROR_INVALID_ARGUMENT) {
        printf("Expected invalid argument error for memory == 0.\n");
        return 17;
    }

    printf("CATMMTEST :: All allocation lifecycle tests passed. 🔥\n");

    return 0;
}
