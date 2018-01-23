// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_PERFLOG_H_
#define STP_BASE_TEST_PERFLOG_H_

#include "Base/Text/StringSpan.h"

namespace stp {

class FilePath;

// Initializes and finalizes the perf log. These functions should be
// called at the beginning and end (respectively) of running all the
// performance tests. The init function returns true on success.
bool InitPerfLog(const FilePath& log_path);
void FinalizePerfLog();

// Writes to the perf result log the given 'value' resulting from the
// named 'test'. The units are to aid in reading the log by people.
void LogPerfResult(StringSpan test_name, double value, StringSpan units);

} // namespace stp

#endif // STP_BASE_TEST_PERFLOG_H_
