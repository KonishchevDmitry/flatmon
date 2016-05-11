#ifndef Util_TypeTraits_hpp
#define Util_TypeTraits_hpp

namespace Util { namespace TypeTraits {

template <typename T> struct removeReference{ typedef T type; };
template <typename T> struct removeReference<T&>{ typedef T type; };

}}

#endif
