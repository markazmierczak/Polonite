// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

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
    stream_.open(
        FilePath(FILE_PATH_LITERAL("/dev/urandom")),
        FileMode::OpenExisting, FileAccess::ReadOnly);
  }

  void read(MutableBufferSpan buffer) { stream_.read(buffer); }

 private:
  FileStream stream_;
};

LazyInstance<URandom>::LeakAtExit g_urandom_fd_instance = LAZY_INSTANCE_INITIALIZER;

} // namespace

void CryptoRandom::generate(MutableBufferSpan buffer) {
  g_urandom_fd_instance->read(buffer);
}

} // namespace stp
