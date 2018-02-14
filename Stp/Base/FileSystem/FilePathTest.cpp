// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/FileSystem/FilePath.h"

#include "Base/Test/GTest.h"
#include "Base/Test/PlatformTest.h"
#include "Base/Text/FormatMany.h"

#if OS(POSIX)
# include "Base/Test/ScopedLocale.h"
#endif

#include "Base/Debug/Log.h"

// This macro helps avoid wrapped lines in the test structs.
#define FPL(x) FILE_PATH_LITERAL(x)

namespace stp {

struct UnaryTestData {
  FilePathLiteral input;
  FilePathLiteral expected;
};

typedef PlatformTest FilePathTest;

TEST_F(FilePathTest, GetDirName) {
  const UnaryTestData cases[] = {
    { FPL(""),              FPL("") },
    { FPL("aa"),            FPL("") },
    { FPL("/aa/bb"),        FPL("/aa") },
    { FPL("/aa/bb/"),       FPL("/aa/bb") },
    { FPL("/aa/bb//"),      FPL("/aa/bb") },
    { FPL("/aa/bb/ccc"),    FPL("/aa/bb") },
    { FPL("/aa"),           FPL("/") },
    { FPL("/aa/"),          FPL("/aa") },
    { FPL("/"),             FPL("/") },
    { FPL("//"),            FPL("//") },
    { FPL("///"),           FPL("//") },
    { FPL("aa/"),           FPL("aa") },
    { FPL("aa/bb"),         FPL("aa") },
    { FPL("aa/bb/"),        FPL("aa/bb") },
    { FPL("aa/bb//"),       FPL("aa/bb") },
    { FPL("aa//bb//"),      FPL("aa//bb") },
    { FPL("aa//bb/"),       FPL("aa//bb") },
    { FPL("aa//bb"),        FPL("aa") },
    { FPL("//aa/bb"),       FPL("//aa") },
    { FPL("//aa/"),         FPL("//aa") },
    { FPL("//aa"),          FPL("//") },
    { FPL("0:"),            FPL("") },
    { FPL("@:"),            FPL("") },
    { FPL("[:"),            FPL("") },
    { FPL("`:"),            FPL("") },
    { FPL("{:"),            FPL("") },
    { FPL("\xB3:"),         FPL("") },
    { FPL("\xC5:"),         FPL("") },
    #if OS(WIN)
    { FPL("\x0143:"),       FPL("") },
    #endif
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:"),            FPL("") },
    { FPL("C:"),            FPL("") },
    { FPL("A:"),            FPL("") },
    { FPL("Z:"),            FPL("") },
    { FPL("a:"),            FPL("") },
    { FPL("z:"),            FPL("") },
    { FPL("c:aa"),          FPL("") },
    { FPL("c:/"),           FPL("") },
    { FPL("c://"),          FPL("") },
    { FPL("c:///"),         FPL("") },
    { FPL("c:/aa"),         FPL("c:/") },
    { FPL("c:/aa/"),        FPL("c:/aa") },
    { FPL("c:/aa/bb"),      FPL("c:/aa") },
    { FPL("c:aa/bb"),       FPL("c:aa") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { FPL("\\aa\\bb"),      FPL("\\aa") },
    { FPL("\\aa\\bb\\"),    FPL("\\aa\\bb") },
    { FPL("\\aa\\bb\\\\"),  FPL("\\aa\\bb") },
    { FPL("\\aa\\bb\\ccc"), FPL("\\aa\\bb") },
    { FPL("\\aa"),          FPL("\\") },
    { FPL("\\aa\\"),        FPL("\\aa") },
    { FPL("\\"),            FPL("\\") },
    { FPL("\\\\"),          FPL("\\\\") },
    { FPL("\\\\\\"),        FPL("\\") },
    { FPL("aa\\"),          FPL("") },
    { FPL("aa\\bb"),        FPL("aa") },
    { FPL("aa\\bb\\"),      FPL("aa\\bb") },
    { FPL("aa\\bb\\\\"),    FPL("aa\\bb") },
    { FPL("aa\\\\bb\\\\"),  FPL("aa\\\\bb") },
    { FPL("aa\\\\bb\\"),    FPL("aa\\\\bb") },
    { FPL("aa\\\\bb"),      FPL("aa") },
    { FPL("\\\\aa\\bb"),    FPL("\\\\aa") },
    { FPL("\\\\aa\\"),      FPL("\\\\aa") },
    { FPL("\\\\aa"),        FPL("\\\\") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:\\"),          FPL("") },
    { FPL("c:\\\\"),        FPL("") },
    { FPL("c:\\\\\\"),      FPL("") },
    { FPL("c:\\aa"),        FPL("c:\\") },
    { FPL("c:\\aa\\"),      FPL("c:\\aa") },
    { FPL("c:\\aa\\bb"),    FPL("c:\\aa") },
    { FPL("c:aa\\bb"),      FPL("c:aa") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    FilePathSpan observed = item.input.GetDirectoryName();
    EXPECT_EQ(item.expected, observed);
  }
}

TEST_F(FilePathTest, GetFileName) {
  const UnaryTestData cases[] = {
    { FPL(""),              FPL("") },
    { FPL("aa"),            FPL("aa") },
    { FPL("/aa/bb"),        FPL("bb") },
    { FPL("/aa/bb/"),       FPL("") },
    { FPL("/aa/bb//"),      FPL("") },
    { FPL("/aa/bb/ccc"),    FPL("ccc") },
    { FPL("/aa"),           FPL("aa") },
    { FPL("/"),             FPL("") },
    { FPL("//"),            FPL("") },
    { FPL("///"),           FPL("") },
    { FPL("aa/"),           FPL("") },
    { FPL("aa/bb"),         FPL("bb") },
    { FPL("aa/bb/"),        FPL("") },
    { FPL("aa/bb//"),       FPL("") },
    { FPL("aa//bb//"),      FPL("") },
    { FPL("aa//bb/"),       FPL("") },
    { FPL("aa//bb"),        FPL("bb") },
    { FPL("//aa/bb"),       FPL("bb") },
    { FPL("//aa/"),         FPL("") },
    { FPL("//aa"),          FPL("aa") },
    { FPL("0:"),            FPL("0:") },
    { FPL("@:"),            FPL("@:") },
    { FPL("[:"),            FPL("[:") },
    { FPL("`:"),            FPL("`:") },
    { FPL("{:"),            FPL("{:") },
    { FPL("\xB3:"),         FPL("\xB3:") },
    { FPL("\xC5:"),         FPL("\xC5:") },
    #if OS(WIN)
    { FPL("\x0143:"),       FPL("\x0143:") },
    #endif
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:"),            FPL("") },
    { FPL("C:"),            FPL("") },
    { FPL("A:"),            FPL("") },
    { FPL("Z:"),            FPL("") },
    { FPL("a:"),            FPL("") },
    { FPL("z:"),            FPL("") },
    { FPL("c:aa"),          FPL("aa") },
    { FPL("c:/"),           FPL("") },
    { FPL("c://"),          FPL("") },
    { FPL("c:///"),         FPL("") },
    { FPL("c:/aa"),         FPL("aa") },
    { FPL("c:/aa/"),        FPL("") },
    { FPL("c:/aa/bb"),      FPL("bb") },
    { FPL("c:aa/bb"),       FPL("bb") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { FPL("\\aa\\bb"),      FPL("bb") },
    { FPL("\\aa\\bb\\"),    FPL("") },
    { FPL("\\aa\\bb\\\\"),  FPL("") },
    { FPL("\\aa\\bb\\ccc"), FPL("ccc") },
    { FPL("\\aa"),          FPL("aa") },
    { FPL("\\"),            FPL("\\") },
    { FPL("\\\\"),          FPL("\\\\") },
    { FPL("\\\\\\"),        FPL("\\") },
    { FPL("aa\\"),          FPL("") },
    { FPL("aa\\bb"),        FPL("bb") },
    { FPL("aa\\bb\\"),      FPL("") },
    { FPL("aa\\bb\\\\"),    FPL("") },
    { FPL("aa\\\\bb\\\\"),  FPL("") },
    { FPL("aa\\\\bb\\"),    FPL("") },
    { FPL("aa\\\\bb"),      FPL("bb") },
    { FPL("\\\\aa\\bb"),    FPL("bb") },
    { FPL("\\\\aa\\"),      FPL("") },
    { FPL("\\\\aa"),        FPL("aa") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:\\"),          FPL("") },
    { FPL("c:\\\\"),        FPL("") },
    { FPL("c:\\\\\\"),      FPL("") },
    { FPL("c:\\aa"),        FPL("aa") },
    { FPL("c:\\aa\\"),      FPL("") },
    { FPL("c:\\aa\\bb"),    FPL("bb") },
    { FPL("c:aa\\bb"),      FPL("bb") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    auto observed = item.input.GetFileName();
    EXPECT_EQ(item.expected, observed);
  }
}

TEST_F(FilePathTest, Combine) {
  struct CombineTestData {
    FilePathLiteral inputs[2];
    FilePathLiteral expected;
  };
  const CombineTestData cases[] = {
    { { FPL(""),           FPL("cc") }, FPL("cc") },
    { { FPL("."),          FPL("ff") }, FPL("./ff") },
    { { FPL("/"),          FPL("cc") }, FPL("/cc") },
    { { FPL("/aa"),        FPL("") },   FPL("/aa/") },
    { { FPL("/aa/"),       FPL("") },   FPL("/aa/") },
    { { FPL("//aa"),       FPL("") },   FPL("//aa/") },
    { { FPL("//aa/"),      FPL("") },   FPL("//aa/") },
    { { FPL("//"),         FPL("aa") }, FPL("//aa") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { { FPL("c:"),         FPL("a") },  FPL("c:a") },
    { { FPL("c:"),         FPL("") },   FPL("c:") },
    { { FPL("c:/"),        FPL("a") },  FPL("c:/a") },
    { { FPL("c://"),       FPL("a") },  FPL("c://a") },
    { { FPL("c:///"),      FPL("a") },  FPL("c:/a") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    // CombineArgs() introduces the default separator character, so these test cases
    // need to be defined with different expected results on platforms that use
    // different default separator characters.
    { { FPL("\\"),         FPL("cc") }, FPL("\\cc") },
    { { FPL("\\aa"),       FPL("") },   FPL("\\aa") },
    { { FPL("\\aa\\"),     FPL("") },   FPL("\\aa") },
    { { FPL("\\\\aa"),     FPL("") },   FPL("\\\\aa") },
    { { FPL("\\\\aa\\"),   FPL("") },   FPL("\\\\aa") },
    { { FPL("\\\\"),       FPL("aa") }, FPL("\\\\aa") },
    { { FPL("/aa/bb"),     FPL("cc") }, FPL("/aa/bb\\cc") },
    { { FPL("/aa/bb/"),    FPL("cc") }, FPL("/aa/bb\\cc") },
    { { FPL("aa/bb/"),     FPL("cc") }, FPL("aa/bb\\cc") },
    { { FPL("aa/bb"),      FPL("cc") }, FPL("aa/bb\\cc") },
    { { FPL("a/b"),        FPL("c") },  FPL("a/b\\c") },
    { { FPL("a/b/"),       FPL("c") },  FPL("a/b\\c") },
    { { FPL("//aa"),       FPL("bb") }, FPL("//aa\\bb") },
    { { FPL("//aa/"),      FPL("bb") }, FPL("//aa\\bb") },
    { { FPL("\\aa\\bb"),   FPL("cc") }, FPL("\\aa\\bb\\cc") },
    { { FPL("\\aa\\bb\\"), FPL("cc") }, FPL("\\aa\\bb\\cc") },
    { { FPL("aa\\bb\\"),   FPL("cc") }, FPL("aa\\bb\\cc") },
    { { FPL("aa\\bb"),     FPL("cc") }, FPL("aa\\bb\\cc") },
    { { FPL("a\\b"),       FPL("c") },  FPL("a\\b\\c") },
    { { FPL("a\\b\\"),     FPL("c") },  FPL("a\\b\\c") },
    { { FPL("\\\\aa"),     FPL("bb") }, FPL("\\\\aa\\bb") },
    { { FPL("\\\\aa\\"),   FPL("bb") }, FPL("\\\\aa\\bb") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { { FPL("c:\\"),       FPL("a") },  FPL("c:\\a") },
    { { FPL("c:\\\\"),     FPL("a") },  FPL("c:\\\\a") },
    { { FPL("c:\\\\\\"),   FPL("a") },  FPL("c:\\a") },
    { { FPL("c:\\"),       FPL("") },   FPL("c:\\") },
    { { FPL("c:\\a"),      FPL("b") },  FPL("c:\\a\\b") },
    { { FPL("c:\\a\\"),    FPL("b") },  FPL("c:\\a\\b") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #else // OS(WIN)
    { { FPL("/aa/bb"),     FPL("cc") }, FPL("/aa/bb/cc") },
    { { FPL("/aa/bb/"),    FPL("cc") }, FPL("/aa/bb/cc") },
    { { FPL("aa/bb/"),     FPL("cc") }, FPL("aa/bb/cc") },
    { { FPL("aa/bb"),      FPL("cc") }, FPL("aa/bb/cc") },
    { { FPL("a/b"),        FPL("c") },  FPL("a/b/c") },
    { { FPL("a/b/"),       FPL("c") },  FPL("a/b/c") },
    { { FPL("//aa"),       FPL("bb") }, FPL("//aa/bb") },
    { { FPL("//aa/"),      FPL("bb") }, FPL("//aa/bb") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { { FPL("c:/"),        FPL("a") },  FPL("c:/a") },
    { { FPL("c:/"),        FPL("") },   FPL("c:/") },
    { { FPL("c:/a"),       FPL("b") },  FPL("c:/a/b") },
    { { FPL("c:/a/"),      FPL("b") },  FPL("c:/a/b") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    FilePath root(item.inputs[0]);
    FilePathSpan leaf = item.inputs[1];
    FilePath observed_path = CombineFilePaths(root, leaf);
    EXPECT_EQ(item.expected, observed_path);

    String ascii = FormattableToString(leaf);
    root.AppendAscii(ascii);
    EXPECT_EQ(item.expected, root);
  }
}

TEST_F(FilePathTest, StripTrailingSeparators) {
  const UnaryTestData cases[] = {
    { FPL(""),              FPL("") },
    { FPL("/"),             FPL("/") },
    { FPL("//"),            FPL("//") },
    { FPL("///"),           FPL("//") },
    { FPL("////"),          FPL("//") },
    { FPL("a/"),            FPL("a") },
    { FPL("a//"),           FPL("a") },
    { FPL("a///"),          FPL("a") },
    { FPL("a////"),         FPL("a") },
    { FPL("/a"),            FPL("/a") },
    { FPL("/a/"),           FPL("/a") },
    { FPL("/a//"),          FPL("/a") },
    { FPL("/a///"),         FPL("/a") },
    { FPL("/a////"),        FPL("/a") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:"),            FPL("c:") },
    { FPL("c:/"),           FPL("c:/") },
    { FPL("c://"),          FPL("c://") },
    { FPL("c:///"),         FPL("c:/") },
    { FPL("c:////"),        FPL("c:/") },
    { FPL("c:/a"),          FPL("c:/a") },
    { FPL("c:/a/"),         FPL("c:/a") },
    { FPL("c:/a//"),        FPL("c:/a") },
    { FPL("c:/a///"),       FPL("c:/a") },
    { FPL("c:/a////"),      FPL("c:/a") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { FPL("\\"),            FPL("\\") },
    { FPL("\\\\"),          FPL("\\\\") },
    { FPL("\\\\\\"),        FPL("\\") },
    { FPL("\\\\\\\\"),      FPL("\\") },
    { FPL("a\\"),           FPL("a") },
    { FPL("a\\\\"),         FPL("a") },
    { FPL("a\\\\\\"),       FPL("a") },
    { FPL("a\\\\\\\\"),     FPL("a") },
    { FPL("\\a"),           FPL("\\a") },
    { FPL("\\a\\"),         FPL("\\a") },
    { FPL("\\a\\\\"),       FPL("\\a") },
    { FPL("\\a\\\\\\"),     FPL("\\a") },
    { FPL("\\a\\\\\\\\"),   FPL("\\a") },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:\\"),          FPL("c:\\") },
    { FPL("c:\\\\"),        FPL("c:\\\\") },
    { FPL("c:\\\\\\"),      FPL("c:\\") },
    { FPL("c:\\\\\\\\"),    FPL("c:\\") },
    { FPL("c:\\a"),         FPL("c:\\a") },
    { FPL("c:\\a\\"),       FPL("c:\\a") },
    { FPL("c:\\a\\\\"),     FPL("c:\\a") },
    { FPL("c:\\a\\\\\\"),   FPL("c:\\a") },
    { FPL("c:\\a\\\\\\\\"), FPL("c:\\a") },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    FilePathSpan observed = item.input;
    observed.StripTrailingSeparators();
    EXPECT_EQ(item.expected, observed);
  }
}

TEST_F(FilePathTest, IsAbsolute) {
  struct UnaryBooleanTestData {
    FilePathLiteral input;
    bool expected;
  };
  const UnaryBooleanTestData cases[] = {
    { FPL(""),       false },
    { FPL("a"),      false },
    { FPL("c:"),     false },
    { FPL("c:a"),    false },
    { FPL("a/b"),    false },
    { FPL("//"),     true },
    { FPL("//a"),    true },
    { FPL("c:a/b"),  false },
    { FPL("?:/a"),   false },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("/"),      false },
    { FPL("/a"),     false },
    { FPL("/."),     false },
    { FPL("/.."),    false },
    { FPL("c:/"),    true },
    { FPL("c:/a"),   true },
    { FPL("c:/."),   true },
    { FPL("c:/.."),  true },
    { FPL("C:/a"),   true },
    { FPL("d:/a"),   true },
    #else // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("/"),      true },
    { FPL("/a"),     true },
    { FPL("/."),     true },
    { FPL("/.."),    true },
    { FPL("c:/"),    false },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { FPL("a\\b"),   false },
    { FPL("\\\\"),   true },
    { FPL("\\\\a"),  true },
    { FPL("a\\b"),   false },
    { FPL("\\\\"),   true },
    { FPL("//a"),    true },
    { FPL("c:a\\b"), false },
    { FPL("?:\\a"),  false },
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("\\"),     false },
    { FPL("\\a"),    false },
    { FPL("\\."),    false },
    { FPL("\\.."),   false },
    { FPL("c:\\"),   true },
    { FPL("c:\\"),   true },
    { FPL("c:\\a"),  true },
    { FPL("c:\\."),  true },
    { FPL("c:\\.."), true },
    { FPL("C:\\a"),  true },
    { FPL("d:\\a"),  true },
    #else // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("\\"),     true },
    { FPL("\\a"),    true },
    { FPL("\\."),    true },
    { FPL("\\.."),   true },
    { FPL("c:\\"),   false },
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    bool observed = item.input.IsAbsolute();
    EXPECT_EQ(item.expected, observed);
  }
}

TEST_F(FilePathTest, GetComponents) {
  const UnaryTestData cases[] = {
    { FPL("//foo/bar/baz/"),          FPL("|//|foo|bar|baz")},
    { FPL("///"),                     FPL("|//")},
    { FPL("/foo//bar//baz/"),         FPL("|/|foo|bar|baz")},
    { FPL("/foo/bar/baz/"),           FPL("|/|foo|bar|baz")},
    { FPL("/foo/bar/baz//"),          FPL("|/|foo|bar|baz")},
    { FPL("/foo/bar/baz///"),         FPL("|/|foo|bar|baz")},
    { FPL("/foo/bar/baz"),            FPL("|/|foo|bar|baz")},
    { FPL("/foo/bar.bot/baz.txt"),    FPL("|/|foo|bar.bot|baz.txt")},
    { FPL("//foo//bar/baz"),          FPL("|//|foo|bar|baz")},
    { FPL("/"),                       FPL("|/")},
    { FPL("foo"),                     FPL("|foo")},
    { FPL(""),                        FPL("")},
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("e:/foo"),                  FPL("|e:|/|foo")},
    { FPL("e:/"),                     FPL("|e:|/")},
    { FPL("e:"),                      FPL("|e:")},
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { FPL("../foo"),                  FPL("|..|foo")},
    { FPL("./foo"),                   FPL("|foo")},
    { FPL("../foo/bar/"),             FPL("|..|foo|bar") },
    { FPL("\\\\foo\\bar\\baz\\"),     FPL("|\\\\|foo|bar|baz")},
    { FPL("\\\\\\"),                  FPL("|\\")},
    { FPL("\\foo\\\\bar\\\\baz\\"),   FPL("|\\|foo|bar|baz")},
    { FPL("\\foo\\bar\\baz\\"),       FPL("|\\|foo|bar|baz")},
    { FPL("\\foo\\bar\\baz\\\\"),     FPL("|\\|foo|bar|baz")},
    { FPL("\\foo\\bar\\baz\\\\\\"),   FPL("|\\|foo|bar|baz")},
    { FPL("\\foo\\bar\\baz"),         FPL("|\\|foo|bar|baz")},
    { FPL("\\foo\\bar/baz\\\\\\"),    FPL("|\\|foo|bar|baz")},
    { FPL("/foo\\bar\\baz"),          FPL("|/|foo|bar|baz")},
    { FPL("\\foo\\bar.bot\\baz.txt"), FPL("|\\|foo|bar.bot|baz.txt")},
    { FPL("\\\\foo\\\\bar\\baz"),     FPL("|\\\\|foo|bar|baz")},
    { FPL("\\"),                      FPL("|\\")},
    #endif // OS(WIN)
  };

  for (const auto& item : cases) {
    List<FilePathChar> observed;
    for (const auto& component : item.input) {
      observed.Append('|');
      observed.Append(component.AsCharactersUnsafe());
    }
    EXPECT_EQ(item.expected.AsCharactersUnsafe(), observed);
  }
}

TEST_F(FilePathTest, Equality) {
  struct EqualityTestData {
    FilePathLiteral inputs[2];
    bool expected;
  };
  const EqualityTestData cases[] = {
    { { FPL("/foo/bar/baz"),  FPL("/foo/bar/baz") },      true},
    { { FPL("/foo/bar"),      FPL("/foo/bar/baz") },      false},
    { { FPL("/foo/bar/baz"),  FPL("/foo/bar") },          false},
    { { FPL("//foo/bar/"),    FPL("//foo/bar/") },        true},
    { { FPL("/foo/bar"),      FPL("/foo2/bar") },         false},
    { { FPL("/foo/bar.txt"),  FPL("/foo/bar") },          false},
    { { FPL("foo/bar"),       FPL("foo/bar") },           true},
    { { FPL("foo/bar"),       FPL("foo/bar/baz") },       false},
    { { FPL(""),              FPL("foo") },               false},
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { { FPL("c:/foo/bar"),    FPL("c:/foo/bar") },        true},
    { { FPL("E:/foo/bar"),    FPL("e:/foo/bar") },        true},
    { { FPL("f:/foo/bar"),    FPL("F:/foo/bar") },        true},
    { { FPL("E:/Foo/bar"),    FPL("e:/foo/bar") },        false},
    { { FPL("f:/foo/bar"),    FPL("F:/foo/Bar") },        false},
    { { FPL("c:/"),           FPL("c:/") },               true},
    { { FPL("c:"),            FPL("c:") },                true},
    { { FPL("c:/foo/bar"),    FPL("d:/foo/bar") },        false},
    { { FPL("c:/foo/bar"),    FPL("D:/foo/bar") },        false},
    { { FPL("C:/foo/bar"),    FPL("d:/foo/bar") },        false},
    { { FPL("c:/foo/bar"),    FPL("c:/foo2/bar") },       false},
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #if OS(WIN)
    { { FPL("\\foo\\bar"),    FPL("\\foo\\bar") },        true},
    { { FPL("\\foo/bar"),     FPL("\\foo/bar") },         true},
    { { FPL("\\foo/bar"),     FPL("\\foo\\bar") },        false},
    { { FPL("\\"),            FPL("\\") },                true},
    { { FPL("\\"),            FPL("/") },                 false},
    { { FPL(""),              FPL("\\") },                false},
    { { FPL("\\foo\\bar"),    FPL("\\foo2\\bar") },       false},
    { { FPL("\\foo\\bar"),    FPL("\\foo\\bar2") },       false},
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { { FPL("c:\\foo\\bar"),    FPL("c:\\foo\\bar") },    true},
    { { FPL("E:\\foo\\bar"),    FPL("e:\\foo\\bar") },    true},
    { { FPL("f:\\foo\\bar"),    FPL("F:\\foo/bar") },     false},
    #endif // HAVE_FILE_PATH_WITH_DRIVE_LETTER
    #endif // OS(WIN)
  };
  for (const auto& item : cases) {
    FilePathSpan a = item.inputs[0];
    FilePathSpan b = item.inputs[1];
    EXPECT_EQ(item.expected, a == b);
  }
  for (const auto& item : cases) {
    FilePath a(item.inputs[0]);
    FilePath b(item.inputs[1]);
    EXPECT_EQ(!item.expected, a != b);
  }
}

TEST_F(FilePathTest, Extension) {
  struct ExtensionTestData {
    FilePathLiteral input;
    StringSpan expected;
  };
  const ExtensionTestData cases[] = {
    #if OS(WIN)
    { FPL("C:\\a\\b\\c.ext"),        ".ext" },
    { FPL("C:\\a\\b\\c."),           "." },
    { FPL("C:\\a\\b\\c"),            "" },
    { FPL("C:\\a\\b\\"),             "" },
    { FPL("C:\\a\\b.\\"),            "." },
    { FPL("C:\\a\\b\\c.ext1.ext2"),  ".ext2" },
    { FPL("C:\\foo.bar\\\\\\"),      ".bar" },
    { FPL("C:\\foo.bar\\.."),        "" },
    { FPL("C:\\foo.bar\\..\\\\"),    "" },
    #endif
    { FPL("/foo/bar/baz.ext"),       ".ext" },
    { FPL("/foo/bar/baz."),          "." },
    { FPL("/foo/bar/baz.."),         "." },
    { FPL("/foo/bar/baz"),           "" },
    { FPL("/foo/bar/"),              "" },
    { FPL("/foo/bar./"),             "" },
    { FPL("/foo/bar/baz.ext1.ext2"), ".ext2" },
    { FPL("/subversion-1.6.12.zip"), ".zip" },
    { FPL("/foo.12345.gz"),          ".gz" },
    { FPL("/foo..gz"),               ".gz" },
    { FPL("."),                      "" },
    { FPL(".."),                     "" },
    { FPL("./foo"),                  "" },
    { FPL("./foo.ext"),              ".ext" },
    { FPL("/foo.ext1/bar.ext2"),     ".ext2" },
    { FPL("/foo.bar////"),           "" },
    { FPL("/foo.bar/.."),            "" },
    { FPL("/foo.bar/..////"),        "" },
    { FPL("/foo.1234.luser.js"),     ".js" },
    { FPL("/user.js"),               ".js" },
    { FPL("/.git"),                  "" },
  };
  for (const auto& item : cases) {
    auto extension = item.input.GetExtension();
    EXPECT_EQ(item.expected, extension);
  }
}

TEST_F(FilePathTest, RemoveExtension) {
  const UnaryTestData cases[] = {
    { FPL(""),                    FPL("") },
    { FPL("."),                   FPL(".") },
    { FPL(".."),                  FPL("..") },
    { FPL("foo.dll"),             FPL("foo") },
    { FPL("./foo.dll"),           FPL("./foo") },
    { FPL("foo..dll"),            FPL("foo.") },
    { FPL("foo"),                 FPL("foo") },
    { FPL("foo."),                FPL("foo") },
    { FPL("foo.."),               FPL("foo.") },
    { FPL("foo.baz.dll"),         FPL("foo.baz") },
    #if OS(WIN)
    { FPL("C:\\foo.bar\\foo"),    FPL("C:\\foo.bar\\foo") },
    { FPL("C:\\foo.bar\\..\\\\"), FPL("C:\\foo.bar\\..\\\\") },
    #endif
    { FPL("/foo.bar/foo"),        FPL("/foo.bar/foo") },
    { FPL("/foo.bar/..////"),     FPL("/foo.bar/..////") },
  };
  for (const auto& item : cases) {
    FilePath path(item.input);
    path.RemoveExtension();
    EXPECT_EQ(item.expected, path);
  }
}

TEST_F(FilePathTest, ChangeExtension) {
  struct ChangeExtensionTestData {
    FilePathLiteral input;
    StringSpan ext;
    FilePathLiteral expected;
  };
  const ChangeExtensionTestData cases[] = {
    { FPL(""),              "",      FPL("") },
    { FPL(""),              "txt",   FPL("") },
    { FPL("."),             "txt",   FPL(".") },
    { FPL(".."),            "txt",   FPL("..") },
    { FPL("."),             "",      FPL(".") },
    { FPL("foo.dll"),       "txt",   FPL("foo.txt") },
    { FPL("./foo.dll"),     "txt",   FPL("./foo.txt") },
    { FPL("foo.dll"),       ".txt",  FPL("foo.txt") },
    { FPL("foo"),           "txt",   FPL("foo.txt") },
    { FPL("foo."),          "txt",   FPL("foo.txt") },
    { FPL("foo"),           ".txt",  FPL("foo.txt") },
    { FPL("foo.baz.dll"),   "txt",   FPL("foo.baz.txt") },
    { FPL("foo.baz.dll"),   ".txt",  FPL("foo.baz.txt") },
    { FPL("foo.dll"),       "",      FPL("foo") },
    { FPL("foo.dll"),       ".",     FPL("foo.") },
    { FPL("foo"),           "",      FPL("foo") },
    { FPL("foo"),           ".",     FPL("foo.") },
    { FPL("foo.baz.dll"),   "",      FPL("foo.baz") },
    { FPL("foo.baz.dll"),   ".",     FPL("foo.baz.") },
    #if OS(WIN)
    { FPL("C:\\foo.bar\\foo"),    "baz", FPL("C:\\foo.bar\\foo.baz") },
    { FPL("C:\\foo.bar\\..\\\\"), "baz", FPL("C:\\foo.bar\\..\\\\") },
    #endif
    { FPL("/foo.bar/foo"),        "baz", FPL("/foo.bar/foo.baz") },
    { FPL("/foo.bar/..////"),     "baz", FPL("/foo.bar/..////") },
  };
  for (const auto& item : cases) {
    FilePath path(item.input);
    path.ReplaceExtension(item.ext);
    EXPECT_EQ(item.expected, path);
  }
}

TEST_F(FilePathTest, MatchesExtension) {
  struct MatchesExtensionTestData {
    FilePathLiteral input;
    StringSpan ext;
    bool expected;
  };
  const MatchesExtensionTestData cases[] = {
    { FPL("foo"),                     "",                    true},
    { FPL("foo"),                     ".",                   false},
    { FPL("foo."),                    "",                    false},
    { FPL("foo."),                    ".",                   true},
    { FPL("foo.txt"),                 ".dll",                false},
    { FPL("foo.txt"),                 ".txt",                true},
    { FPL("foo.txt.dll"),             ".txt",                false},
    { FPL("foo.txt.dll"),             ".dll",                true},
    { FPL("foo.TXT"),                 ".txt",                true},
    { FPL("foo.txt"),                 ".TXT",                true},
    { FPL("foo.tXt"),                 ".txt",                true},
    { FPL("foo.txt"),                 ".tXt",                true},
    { FPL("foo.tXt"),                 ".TXT",                true},
    { FPL("foo.tXt"),                 ".tXt",                true},
    #if HAVE_FILE_PATH_WITH_DRIVE_LETTER
    { FPL("c:/foo.txt.dll"),          ".txt",                false},
    { FPL("c:/foo.txt"),              ".txt",                true},
    #endif
    #if OS(WIN)
    { FPL("c:\\bar\\foo.txt.dll"),    ".txt",                false},
    { FPL("c:\\bar\\foo.txt"),        ".txt",                true},
    #endif
    { FPL("/bar/foo.txt.dll"),        ".txt",                false},
    { FPL("/bar/foo.txt"),            ".txt",                true},
  };

  for (const auto& item : cases) {
    FilePathSpan path = item.input;
    EXPECT_EQ(item.expected, path.MatchesExtension(item.ext));
  }
}

TEST_F(FilePathTest, FromToString) {
  struct Utf8TestData {
    FilePathLiteral path;
    StringSpan string;
  };
  const Utf8TestData cases[] = {
    { FPL("foo.txt"), "foo.txt" },
    // "aeo" with accents. Use http://0xcc.net/jsescape/ to decode them.
    { FPL("\u00E0\u00E8\u00F2.txt"), "\xC3\xA0\xC3\xA8\xC3\xB2.txt" },
    // Full-width "ABC".
    { FPL("\uFF21\uFF22\uFF23.txt"), "\xEF\xBC\xA1\xEF\xBC\xA2\xEF\xBC\xA3.txt" },
  };

  #if !HAVE_UTF8_NATIVE_VALIDATION && OS(LINUX)
  ScopedLocale locale("en_US.UTF-8");
  #endif

  for (const auto& item : cases) {
    FilePath from_utf8 = FilePath::FromString(item.string);
    EXPECT_EQ(item.path, from_utf8);
    EXPECT_EQ(item.string, FormattableToString(item.path));
  }
}

#if OS(WIN)
TEST_F(FilePathTest, NormalizeSeparators) {
  const UnaryTestData cases[] = {
    { FPL("foo/bar"), FPL("foo\\bar") },
    { FPL("foo/bar\\betz"), FPL("foo\\bar\\betz") },
    { FPL("foo\\bar"), FPL("foo\\bar") },
    { FPL("foo\\bar/betz"), FPL("foo\\bar\\betz") },
    { FPL("foo"), FPL("foo") },
    // Trailing slashes don't automatically get stripped.  That's what
    // StripTrailingSeparators() is for.
    { FPL("foo\\"), FPL("foo\\") },
    { FPL("foo/"), FPL("foo\\") },
    { FPL("foo/bar\\"), FPL("foo\\bar\\") },
    { FPL("foo\\bar/"), FPL("foo\\bar\\") },
    { FPL("foo/bar/"), FPL("foo\\bar\\") },
    { FPL("foo\\bar\\"), FPL("foo\\bar\\") },
    { FPL("\\foo/bar"), FPL("\\foo\\bar") },
    { FPL("/foo\\bar"), FPL("\\foo\\bar") },
    { FPL("c:/foo/bar/"), FPL("c:\\foo\\bar\\") },
    { FPL("/foo/bar/"), FPL("\\foo\\bar\\") },
    { FPL("\\foo\\bar\\"), FPL("\\foo\\bar\\") },
    { FPL("c:\\foo/bar"), FPL("c:\\foo\\bar") },
    { FPL("//foo\\bar\\"), FPL("\\\\foo\\bar\\") },
    { FPL("\\\\foo\\bar\\"), FPL("\\\\foo\\bar\\") },
    { FPL("//foo\\bar\\"), FPL("\\\\foo\\bar\\") },
    // This method does not normalize the number of path separators.
    { FPL("foo\\\\bar"), FPL("foo\\\\bar") },
    { FPL("foo//bar"), FPL("foo\\\\bar") },
    { FPL("foo/\\bar"), FPL("foo\\\\bar") },
    { FPL("foo\\/bar"), FPL("foo\\\\bar") },
    { FPL("///foo\\\\bar"), FPL("\\\\\\foo\\\\bar") },
    { FPL("foo//bar///"), FPL("foo\\\\bar\\\\\\") },
    { FPL("foo/\\bar/\\"), FPL("foo\\\\bar\\\\") },
    { FPL("/\\foo\\/bar"), FPL("\\\\foo\\\\bar") },
  };
  for (const auto& item : cases) {
    FilePath observed(item.input);
    observed.NormalizeSeparators();
    EXPECT_EQ(item.expected, observed);
  }
}
#endif

} // namespace stp
