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

        // Calculates median of the items
        ValueType median() {
            UTIL_ASSERT(!empty());

            ValueType itemsCopy[size_];

            if(size_ == capacity()) {
                memcpy(itemsCopy, items_, size_ * sizeof(ValueType));
            } else if(start_ + size_ <= capacity()){
                memcpy(itemsCopy, items_ + start_, size_ * sizeof(ValueType));
            } else {
                size_t partSize = capacity() - start_;
                memcpy(itemsCopy, items_ + start_, partSize * sizeof(ValueType));
                memcpy(itemsCopy + partSize, items_, (size_ - partSize) * sizeof(ValueType));
            }

            Util::sort(itemsCopy, itemsCopy + size_);

            if(size_ % 2)
                return itemsCopy[size_ / 2];
            else
                return (itemsCopy[size_ / 2] + itemsCopy[size_ / 2 - 1]) / 2;
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

}

#endif
