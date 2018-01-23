// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODECDATABASE_H_
#define STP_BASE_TEXT_CODEC_TEXTCODECDATABASE_H_

#include "Base/Containers/FlatMap.h"
#include "Base/Sync/ReadWriteLock.h"
#include "Base/Text/Codec/TextCodec.h"
#include "Base/Containers/List.h"

namespace stp {

class TextCodecDatabase {
 public:
  explicit TextCodecDatabase() {}

  TextCodec GetForName(StringSpan name) {
    AutoReadLock auto_lock(cache_lock_);
    const TextCodec* codec_ptr = cache_.TryGet(name);
    return codec_ptr ? *codec_ptr : TextCodec();
  }

  void SetForName(StringSpan name, TextCodec codec) {
    AutoWriteLock auto_lock(cache_lock_);
    cache_.Set(name, codec);
  }

 private:
  ReadWriteLock cache_lock_;
  FlatMap<String, TextCodec> cache_;
};

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODECDATABASE_H_
