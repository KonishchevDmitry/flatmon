#ifndef Util_LinkedNodeList_hpp
#define Util_LinkedNodeList_hpp

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <cstddef>
#endif

#include "Assertion.hpp"
#include "TypeTraits.hpp"

namespace Util {

template <class T>
class LinkedNodeList: TypeTraits::NonCopyable {
    public:
        T* prev() const {
            return prev_;
        }

        T* next() const {
            return next_;
        }

        void insert(T* node) {
            UTIL_ASSERT(!node->prev_ && !node->next_);

            node->prev_ = static_cast<T*>(this);
            node->next_ = this->next_;

            if(this->next_)
                this->next_->prev_ = node;

            this->next_ = node;
        }

        bool remove(T** next) {
            bool first = prev_ == nullptr;
            *next = next_;

            if(prev_)
                prev_->next_ = next_;

            if(next_)
                next_->prev_ = prev_;

            prev_ = nullptr;
            next_ = nullptr;

            return first;
        }

    protected:
        LinkedNodeList(): prev_(nullptr), next_(nullptr) {
        }

    private:
        T* prev_;
        T* next_;
};

}

#endif
