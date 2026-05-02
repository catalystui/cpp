#ifndef CATMMLIB_HPP
#define CATMMLIB_HPP
#include "CATCRLIB.hpp"
#include "CATMMLIB.h"
namespace catalyst {

typedef ::CATALYST_MEMORY_HEADER MEMORY_HEADER;

inline void alloc(void** memory, NUINT size, RESULT* result) {
    ::catmmAlloc(memory, size, result);
}

inline void realloc(void** memory, NUINT size, RESULT* result) {
    ::catmmRealloc(memory, size, result);
}

inline void free(void* memory, RESULT* result) {
    ::catmmFree(memory, result);
}

} /* namespace catalyst */
#endif /* CATMMLIB_HPP */
