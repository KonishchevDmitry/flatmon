#ifndef Util_CycleBuffer_h
#define Util_CycleBuffer_h

#include <stdlib.h>

#include "Assertion.h"

namespace Util {

template<typename T, size_t N>
class CycleBuffer {
    public:
        typedef T ValueType;

        CycleBuffer(): start_(0), size_(0) {
        }

        void add(T value) {
            if(size_ < capacity()) {
                ++size_;
            } else {
                start_ = (start_ + 1) % capacity();
            }

            items_[(start_ + size_ - 1) % capacity()] = value;
        }

        // Calculates median of the items
        ValueType median() {
            UTIL_ASSERT(!empty());

            ValueType items_copy[size_];

            if(size_ == capacity()) {
                memcpy(items_copy, items_, size_ * sizeof(ValueType));
            } else if(start_ + size_ <= capacity()){
                memcpy(items_copy, items_ + start_, size_ * sizeof(ValueType));
            } else {
                size_t part_size = capacity() - start_;
                memcpy(items_copy, items_ + start_, part_size * sizeof(ValueType));
                memcpy(items_copy + part_size, items_, (size_ - part_size) * sizeof(ValueType));
            }

            qsort(items_copy, size_, sizeof(ValueType), compare_values_);

            if(size_ % 2)
                return items_copy[size_ / 2];
            else
                return (items_copy[size_ / 2] + items_copy[size_ / 2 - 1]) / 2;
        }

        size_t capacity() const {
            return N;
        }

        size_t size() const {
            return size_;
        }

        bool empty() const {
            return size() == 0;
        }

        bool full() const {
            return size() == capacity();
        }

        // FIXME: HERE
    protected:
        T items_[N];
        size_t start_;
        size_t size_;

        static int compare_values_(const void* a_ptr, const void* b_ptr) {
            ValueType a = *reinterpret_cast<const ValueType*>(a_ptr);
            ValueType b = *reinterpret_cast<const ValueType*>(b_ptr);

            if(a < b)
                return -1;
            else if(a > b)
                return 1;
            else
                return 0;
        }
};

}

#endif
