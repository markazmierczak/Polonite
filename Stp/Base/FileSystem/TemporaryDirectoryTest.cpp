// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/TemporaryDirectory.h"

#include "Base/FileSystem/File.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(TemporaryDirectory, FullPath) {
  FilePath test_path;
  CreateNewTempDirectory(FILE_PATH_LITERAL("scoped_temp_dir"), &test_path);

  // Against an existing dir, it should get destroyed when leaving scope.
  EXPECT_TRUE(DirectoryExists(test_path));
  {
    TemporaryDirectory dir;
    EXPECT_TRUE(dir.set(test_path));
    EXPECT_TRUE(dir.isValid());
  }
  EXPECT_FALSE(DirectoryExists(test_path));

  {
    TemporaryDirectory dir;
    EXPECT_TRUE(dir.set(test_path));
    // Now the dir doesn't exist, so ensure that it gets created.
    EXPECT_TRUE(DirectoryExists(test_path));
    // When we call release(), it shouldn't get destroyed when leaving scope.
    FilePath path = dir.take();
    EXPECT_EQ(path.chars, test_path.chars);
    EXPECT_FALSE(dir.isValid());
  }
  EXPECT_TRUE(DirectoryExists(test_path));

  // Clean up.
  {
    TemporaryDirectory dir;
    EXPECT_TRUE(dir.set(test_path));
  }
  EXPECT_FALSE(DirectoryExists(test_path));
}

TEST(TemporaryDirectory, TempDir) {
  // In this case, just verify that a directory was created and that it's a
  // child of TempDir.
  FilePath test_path;
  {
    TemporaryDirectory dir;
    EXPECT_TRUE(dir.create());
    test_path = dir.path();
    EXPECT_TRUE(DirectoryExists(test_path));
    FilePath tmp_dir;
    EXPECT_TRUE(GetTempDir(&tmp_dir));
    EXPECT_TRUE(test_path.chars.find(tmp_dir.chars) != std::string::npos);
  }
  EXPECT_FALSE(DirectoryExists(test_path));
}

TEST(TemporaryDirectory, UniqueTempDirUnderPath) {
  // Create a path which will contain a unique temp path.
  FilePath base_path;
  ASSERT_TRUE(CreateNewTempDirectory(FILE_PATH_LITERAL("base_dir"),
                                           &base_path));

  FilePath test_path;
  {
    TemporaryDirectory dir;
    EXPECT_TRUE(dir.createUnder(base_path));
    test_path = dir.path();
    EXPECT_TRUE(DirectoryExists(test_path));
    EXPECT_TRUE(base_path.IsParent(test_path));
    EXPECT_TRUE(test_path.chars.find(base_path.chars) != std::string::npos);
  }
  EXPECT_FALSE(DirectoryExists(test_path));
  DeleteFile(base_path, true);
}

TEST(TemporaryDirectory, MultipleInvocations) {
  TemporaryDirectory dir;
  EXPECT_TRUE(dir.create());
  EXPECT_FALSE(dir.create());
  EXPECT_TRUE(dir.remove());
  EXPECT_TRUE(dir.create());
  EXPECT_FALSE(dir.create());
  TemporaryDirectory other_dir;
  EXPECT_TRUE(other_dir.set(dir.take()));
  EXPECT_TRUE(dir.create());
  EXPECT_FALSE(dir.create());
  EXPECT_FALSE(other_dir.create());
}

#if OS(WIN)
TEST(TemporaryDirectory, LockedTempDir) {
  TemporaryDirectory dir;
  EXPECT_TRUE(dir.create());
  File file(dir.path().append(FILE_PATH_LITERAL("temp")),
                  File::FLAG_CREATE_ALWAYS | File::FLAG_WRITE);
  EXPECT_TRUE(file.isValid());
  EXPECT_EQ(File::FILE_OK, file.error_details());
  EXPECT_FALSE(dir.remove());  // We should not be able to delete.
  EXPECT_FALSE(dir.path().empty());  // We should still have a valid path.
  file.Close();
  // Now, we should be able to delete.
  EXPECT_TRUE(dir.remove());
}
#endif // OS(WIN)

} // namespace stp
