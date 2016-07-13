#include "Core.hpp"

extern int* __brkval;
extern int __heap_start;

size_t getStackFreeMemorySize() {
    char var;
    auto stackAddress = reinterpret_cast<uintptr_t>(&var);
    auto heapAddress = reinterpret_cast<uintptr_t>(__brkval ? __brkval : &__heap_start);
    return heapAddress < stackAddress ? stackAddress - heapAddress : 0;
}
