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

            (*this)[size_ - 1] = value;
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

        ValueType& operator[](size_t pos) {
            return items_[(start_ + pos) % capacity()];
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

        ValueType maxValue() {
            // Note: The method may be implemented in a more readable way, but the following code is optimized for
            // perfomance.

            UTIL_ASSERT(!this->empty());

            ValueType* begin = this->items_;
            ValueType* end = begin + N;

            ValueType* first_begin = begin + this->start_;
            ValueType* first_end = first_begin + this->size_;

            ValueType maxValue = *first_begin++;

            if(first_end > end) {
                ValueType* second_begin = begin;
                ValueType* second_end = begin + (first_end - end);
                first_end = end;

                while(second_begin < second_end) {
                    ValueType value = *second_begin++;
                    if(value > maxValue)
                        maxValue = value;
                }
            }

            while(first_begin < first_end) {
                ValueType value = *first_begin++;
                if(value > maxValue)
                    maxValue = value;
            }

            return maxValue;
        }
};

}

#endif
