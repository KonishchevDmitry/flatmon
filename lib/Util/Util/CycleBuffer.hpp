#ifndef Util_CycleBuffer_hpp
#define Util_CycleBuffer_hpp

#include <stdlib.h>

#include "Assertion.hpp"
#include "Core.hpp"
#include "Sort.hpp"

namespace Util {

template <typename T, size_t N>
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

    protected:
        ValueType items_[N];
        size_t start_;
        size_t size_;
};

template <typename T, size_t N>
class NumericCycleBuffer: public CycleBuffer<T, N> {
    public:
        using typename CycleBuffer<T, N>::ValueType;

        // Calculates median of the items
        ValueType median() {
            UTIL_ASSERT(!this->empty());

            ValueType itemsCopy[this->size_];

            if(this->size_ == this->capacity()) {
                memcpy(itemsCopy, this->items_, this->size_ * sizeof(ValueType));
            } else if(this->start_ + this->size_ <= this->capacity()){
                memcpy(itemsCopy, this->items_ + this->start_, this->size_ * sizeof(ValueType));
            } else {
                size_t partSize = this->capacity() - this->start_;
                memcpy(itemsCopy, this->items_ + this->start_, partSize * sizeof(ValueType));
                memcpy(itemsCopy + partSize, this->items_, (this->size_ - partSize) * sizeof(ValueType));
            }

            Util::sort(itemsCopy, itemsCopy + this->size_);

            if(this->size_ % 2)
                return itemsCopy[this->size_ / 2];
            else
                return (itemsCopy[this->size_ / 2] + itemsCopy[this->size_ / 2 - 1]) / 2;
        }
};

}

#endif
