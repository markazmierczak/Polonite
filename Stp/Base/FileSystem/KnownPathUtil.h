// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_KNOWNPATHUTIL_H_
#define STP_BASE_FS_KNOWNPATHUTIL_H_

#include "Base/Containers/MapFwd.h"
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
BASE_EXPORT FilePath ResolveInternal(Key& key, ProvideType provider, bool directory, Option option);
}

inline FilePath ResolveFile(Key& key, ProvideType provider, Option option) {
  return detail::ResolveInternal(key, provider, false, option);
}

inline FilePath ResolveDirectory(Key& key, ProvideType provider, Option option) {
  return detail::ResolveInternal(key, provider, true, option);
}

class BASE_EXPORT NotFoundException : public Exception {
 public:
  explicit NotFoundException(FilePath path) noexcept;
  StringSpan getName() const noexcept override;

  const FilePath& GetPath() const noexcept { return path_; }

 private:
  FilePath path_;
};

} // namespace known_path
} // namespace stp

#endif // STP_BASE_FS_KNOWNPATHUTIL_H_
