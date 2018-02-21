// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_AUTORESET_H_
#define STP_BASE_UTIL_AUTORESET_H_

#include "Base/Type/Variable.h"

// AutoReset<> is useful for setting a variable to a new value only within
// a particular scope. An AutoReset<> object resets a variable to its
// original value upon destruction, making it an alternative to writing
// "var = false;" or "var = old_val;" at all of a block's exit points.
//
// This should be obvious, but note that an AutoReset<> instance should
// have a shorter lifetime than its scoped_variable, to prevent invalid memory
// writes when the AutoReset<> object is destroyed.

namespace stp {

template<typename T>
class AutoReset {
 public:
  template<typename U>
  AutoReset(T* var, U&& new_value)
      : scoped_variable_(var),
        original_value_(exchange(*var, forward<U>(new_value))) {
  }

  ~AutoReset() {
    if (scoped_variable_)
      swap(*scoped_variable_, original_value_);
  }

  // The change will be permanent - no rollback happen at destruction.
  void persist() { scoped_variable_ = nullptr; }

 private:
  T* scoped_variable_;
  T original_value_;

  DISALLOW_COPY_AND_ASSIGN(AutoReset);
};

} // namespace stp

#endif // STP_BASE_UTIL_AUTORESET_H_
