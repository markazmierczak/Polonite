// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/List.h"

namespace stp {

/** \n List
 * A List is a resizable array (std::vector equivalent).
 * https://en.wikipedia.org/wiki/List_(abstract_data_type)
 */

/** \fn void List::removeLast()
 * Removes the last item from the list.
 * The list must not be empty.
 */

/** \fn void List::removeAt(int at)
 * Removes an item at the specified index.
 */

/** \fn void List::removeRange(int at, int n)
 * Removes a range of items.
 * The given slice must be enclosed by [0..size).
 */

} // namespace stp
