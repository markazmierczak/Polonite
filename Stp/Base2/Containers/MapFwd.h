// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_MAPFWD_H_
#define STP_BASE_CONTAINERS_MAPFWD_H_

#include "Base/Containers/ListFwd.h"

namespace stp {

template<typename K, typename T>
class KeyValuePair;

template<typename K, typename T, template<class TItem> class TList = List>
class FlatMap;

template<typename T, template<class TItem> class TList = List>
class FlatSet;

template<typename K, typename T>
class HashMap;

template<typename T>
class HashSet;

} // namespace stp

#endif // STP_BASE_CONTAINERS_MAPFWD_H_
