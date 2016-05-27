#ifndef Util_Sort_hpp
#define Util_Sort_hpp

#include <stdlib.h>

#include "Core.hpp"
#include "TypeTraits.hpp"

namespace Util {

template <typename T>
int compareItems(const void* aPtr, const void* bPtr) {
    T a = *reinterpret_cast<const T*>(aPtr);
    T b = *reinterpret_cast<const T*>(bPtr);

    if(a < b)
        return -1;
    else if(a > b)
        return 1;
    else
        return 0;
}

template <typename Iterator>
void sort(Iterator begin, Iterator end) {
    size_t size = end - begin;

    if(size) {
        void* items = &(*begin);
        size_t itemSize = sizeof *begin;
        int (*compareFunc)(const void*, const void*) = \
            compareItems<typename Util::TypeTraits::removeReference<decltype(*begin)>::type>;
        qsort(items, size, itemSize, compareFunc);
    }
}

}

#endif
