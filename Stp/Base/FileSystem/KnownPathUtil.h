// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_KNOWNPATHUTIL_H_
#define STP_BASE_FS_KNOWNPATHUTIL_H_

#include "Base/FileSystem/FilePath.h"

namespace stp {
namespace known_path {

using Key = int;

enum Option {
  NotValidated,
  EnsureExists,
  EnsureCreated, // only for directory variant
};

typedef FilePath (*ProvideType)();

namespace detail {
BASE_EXPORT FilePath resolveInternal(Key& key, ProvideType provider, bool directory, Option option);
}

inline FilePath resolveFile(Key& key, ProvideType provider, Option option) {
  return detail::resolveInternal(key, provider, false, option);
}

inline FilePath resolveDirectory(Key& key, ProvideType provider, Option option) {
  return detail::resolveInternal(key, provider, true, option);
}

} // namespace known_path
} // namespace stp

#endif // STP_BASE_FS_KNOWNPATHUTIL_H_
