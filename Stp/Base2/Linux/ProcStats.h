// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_LINUX_PROCSTATS_H_
#define STP_BASE_LINUX_PROCSTATS_H_

#include "Base/Linux/ProcCommon.h"
#include "Base/Text/StringList.h"

namespace stp {
namespace linux {

class ProcStats : public ProcCommon {
 public:
  enum Field : int {
    // See man(5) proc for details.
    SelfID = 1,
    ExecutableFilename = 2,
    State = 3,
    ParentID = 4,
    GroupID = 5,
    ScheduledUserTime = 14,
    ScheduledKernelTime = 15,
    NumberOfThreads = 20,
    StartTime = 22,
    VirtualMemorySize = 23,
    ResidentSetSize = 24,
  };

  class Reader {
   public:
    Reader() {}

    bool Open(NativeProcessHandle pid);

    bool OpenFile(const FilePath& path);

    bool Parse(StringSpan content);

    StringSpan GetField(Field field) const;

    template<typename T>
    bool GetFieldAsInt(Field field, T& value);

   private:
    String content_;

    List<StringSpan> list_;
  };
};

template<typename T>
inline bool ProcStats::Reader::GetFieldAsInt(Field field, T& value) {
  return TryParse(GetField(field), value);
}

} // namespace linux
} // namespace stp

#endif // STP_BASE_LINUX_PROCSTATS_H_
