// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Library.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(LibraryTest, LoadFailure) {
  FilePath DummyLibraryPath(FILE_PATH_LITERAL("dummy_library"));

  Library lib;
  ASSERT_FALSE(lib.TryLoad(DummyLibraryPath));
}

TEST(LibraryTest, DecorateName) {
  const StringSpan ExpectedName =
      #if OS(IOS)
      "mylib";
      #elif OS(MAC)
      "libmylib.dylib";
      #elif OS(POSIX)
      "libmylib.so";
      #elif OS(WIN)
      "mylib.dll";
      #endif

  EXPECT_EQ(ExpectedName, Library::DecorateName("mylib"));
}

} // namespace stp
