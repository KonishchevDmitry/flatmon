#ifndef Util_TypeTraits_h
#define Util_TypeTraits_h

namespace Util { namespace TypeTraits {

template <typename T> struct removeReference{ typedef T type; };
template <typename T> struct removeReference<T&>{ typedef T type; };

}}

#endif
