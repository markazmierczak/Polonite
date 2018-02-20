// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/KnownPathUtil.h"

#include "Base/Compiler/Lsan.h"
#include "Base/Containers/HashMap.h"
#include "Base/FileSystem/Directory.h"
#include "Base/FileSystem/File.h"
#include "Base/Thread/Lock.h"
#include "Base/Type/Hashable.h"

namespace stp {
namespace known_path {

namespace {

typedef HashMap<int, FilePath> PathMap;

constexpr bool CacheDisabled = false;

BasicLock g_database_lock = BASIC_LOCK_INITIALIZER;
PathMap* g_database = nullptr;
int g_next_key = 0;

} // namespace

namespace detail {

FilePath ResolveInternal(
    int& key, ProvideType provider, bool directory, Option option) {
  FilePath path;

  bool resolved = false;
  if (!CacheDisabled) {
    AutoLock scoped_lock(&g_database_lock);
    if (key == 0) {
      key = g_next_key++;
      if (key == 0) {
        ANNOTATE_SCOPED_MEMORY_LEAK;
        g_database = new PathMap();
        key = g_next_key++;
      }
    } else {
      resolved = true;
      path = (*g_database)[key];
    }
    ASSERT(key != 0);
  }
  if (resolved)
    return path;

  path = (*provider)();

  if (!path.isEmpty() && !path.IsAbsolute())
    path = File::MakeAbsolutePath(path);

  if (option != NotValidated) {
    if (option == EnsureCreated) {
      ASSERT(directory);
      Directory::Create(path);
    } else if (option == EnsureExists) {
      bool exists;
      if (directory) {
        exists = Directory::Exists(path);
      } else {
        exists = File::Exists(path);
      }
      if (!exists)
        throw NotFoundException(move(path));
    }
  }

  if (!CacheDisabled) {
    AutoLock scoped_lock(&g_database_lock);
    // Must be tryAdd() instead of add() - other thread might already resolve this path.
    g_database->tryAdd(key, path);
  }
  return path;
}

} // namespace detail

NotFoundException::NotFoundException(FilePath path) noexcept
    : path_(move(path)) {
}

StringSpan NotFoundException::getName() const noexcept {
  return "known_path::NotFoundException";
}

} // namespace known_path
} // namespace stp
