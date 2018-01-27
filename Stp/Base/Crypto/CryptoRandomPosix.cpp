// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Crypto/CryptoRandom.h"

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

  void Read(MutableBufferSpan buffer) { stream_.Read(buffer); }

 private:
  FileStream stream_;
};

LazyInstance<URandom>::LeakAtExit g_urandom_fd_instance = LAZY_INSTANCE_INITIALIZER;

} // namespace

void CryptoRandom::Fill(MutableBufferSpan buffer) noexcept {
  g_urandom_fd_instance->Read(buffer);
}

} // namespace stp
