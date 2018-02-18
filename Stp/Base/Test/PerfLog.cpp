// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Test/PerfLog.h"

#include "Base/Io/FileStream.h"
#include "Base/Io/StreamWriter.h"
#include "Base/Text/FormatMany.h"

namespace stp {

static FileStream* g_perf_log_stream = nullptr;

bool InitPerfLog(const FilePath& log_file) {
  ASSERT(!g_perf_log_stream);
  g_perf_log_stream = new FileStream();
  return !!g_perf_log_stream->TryCreate(log_file);
}

void FinalizePerfLog() {
  ASSERT(g_perf_log_stream);
  delete g_perf_log_stream;
  g_perf_log_stream = nullptr;
}

void LogPerfResult(StringSpan test_name, double value, StringSpan units) {
  ASSERT(g_perf_log_stream);

  // FIXME
//  StreamWriter out(g_perf_log_stream);
//  out.format("{}\t{}\t{}\n", test_name, value, units);
//  out.Flush();
}

} // namespace stp
