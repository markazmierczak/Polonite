// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Random/CryptoRandom.h"

#include "Base/Io/FileStream.h"
#include "Base/Util/LazyInstance.h"

namespace stp {

namespace {

// We keep the file descriptor for /dev/urandom around so we don't need to
// reopen it (which is expensive).
class URandom {
 public:
  URandom() {
    stream_.Open(
        FilePath(FILE_PATH_LITERAL("/dev/urandom")),
        FileMode::OpenExisting, FileAccess::ReadOnly);
  }

  void Read(byte_t* dst, int n) { stream_.Read(dst, n); }

 private:
  FileStream stream_;
};

LazyInstance<URandom>::LeakAtExit g_urandom_fd_instance = LAZY_INSTANCE_INITIALIZER;

} // namespace

void CryptoRandom::NextBytes(byte_t* output, int output_length) {
  g_urandom_fd_instance->Read(output, output_length);
}

} // namespace stp
