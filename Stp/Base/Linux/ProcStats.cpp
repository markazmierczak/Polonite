// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Linux/ProcStats.h"

namespace stp {
namespace linux {

bool ProcStats::Reader::Open(NativeProcessHandle pid) {
  return OpenFile(DirectoryForProcess(pid));
}

bool ProcStats::Reader::OpenFile(const FilePath& path) {
  if (!ReadFileToString(path, &content_)) {
    LOGF("failed to read %" PRIsFP, path.c_str());
    return false;
  }
  return Parse(content_);
}

bool ProcStats::Reader::Parse(StringSpan content_string) {
  // Reader object cannot be reused.
  ASSERT(content_.isEmpty());
  content_ = std::move(content_string);

  const char* begin = content_.data();
  const char* end = begin + content_.size();

  if (begin == end)
    return false;

  // The stat file is formatted as:
  // pid (process name) data1 data2 .... dataN
  // Look for the closing paren by scanning backwards, to avoid being fooled by
  // processes with ')' in the name.
  size_t open_parens_idx = content.find(" (");
  size_t close_parens_idx = content.find(") ");
  if (open_parens_idx > close_parens_idx ||
      close_parens_idx == std::string::npos) {
    ASSERTF(false, "failed to find matched parens in '%s'", content_string.c_str());
    return false;
  }

  // FIXME parse PID
  list_.push_back(content.substr(0, open_parens_idx));

  open_parens_idx++;

  // PID.
  proc_stats->push_back(stats_data.substr(0, open_parens_idx));
  // Process name without parentheses.
  proc_stats->push_back(
      stats_data.substr(open_parens_idx + 1,
                        close_parens_idx - (open_parens_idx + 1)));

  // Split the rest.
  std::vector<std::string> other_stats = SplitString(
      stats_data.substr(close_parens_idx + 2), " ",
      TRIM_WHITESPACE, SPLIT_WANT_ALL);
  for (size_t i = 0; i < other_stats.size(); ++i)
    proc_stats->push_back(other_stats[i]);
  return true;
}

StringSpan ProcStats::Reader::GetField(Field field) const {
  return field < list_.size() ? list_[field] : StringSpan();
}

} // namespace linux
} // namespace stp
