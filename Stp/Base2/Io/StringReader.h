// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_STRINGREADER_H_
#define STP_BASE_IO_STRINGREADER_H_

#include "Base/Io/TextReader.h"

namespace stp {

class BASE_EXPORT StringReader : public TextReader {
 public:
  explicit StringReader(StringSpan string) : string_(string) {}

 protected:
  int32_t OnPeek() override;
  int32_t OnRead() override;
  int OnRead(char* dst, int count) override;

 private:
  int32_t OnReadInternal(bool advance);

  StringSpan string_;
};

} // namespace stp

#endif // STP_BASE_IO_STRINGREADER_H_
