// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_INITIALIZERLIST_H_
#define STP_BASE_CONTAINERS_INITIALIZERLIST_H_

#include <initializer_list>

namespace stp {

template<typename T>
using InitializerList = std::initializer_list<T>;

} // namespace stp

#endif // STP_BASE_CONTAINERS_INITIALIZERLIST_H_
