// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Test/PerfTest.h"

#include "Base/Text/Format.h"

#include <stdio.h>

namespace stp {
namespace perf_test {

namespace {

String ResultsToString(
    const String& measurement, const String& modifier, const String& trace,
    const String& values, const String& prefix, const String& suffix,
    const String& units,
    bool important) {
  // <*>RESULT <graph_name>: <trace_name>= <value> <units>
  // <*>RESULT <graph_name>: <trace_name>= {<mean>, <std deviation>} <units>
  // <*>RESULT <graph_name>: <trace_name>= [<value>,value,value,...,] <units>
  return StringFormatMany(
      "{}RESULT {}{}: {}= {}{}{} {}\n",
      important ? "*" : "", measurement, modifier, trace, prefix, values, suffix, units);
}

void PrintResultsImpl(
    const String& measurement, const String& modifier, const String& trace,
    const String& values,
    const String& prefix, const String& suffix,
    const String& units,
    bool important) {
  fflush(stdout);
  printf("%s", ToNullTerminated(ResultsToString(
      measurement, modifier, trace, values,
      prefix, suffix, units, important)));
  fflush(stdout);
}

} // namespace

void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    size_t value, const String& units,
    bool important) {
  PrintResultsImpl(
      measurement, modifier, trace,
      FormattableToString(static_cast<unsigned int>(value), "d"),
      String(), String(), units, important);
}

void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    double value, const String& units, bool important) {
  PrintResultsImpl(
      measurement, modifier, trace,
      FormattableToString(value), String(), String(), units,
      important);
}

void AppendResult(
    String& output, const String& measurement, const String& modifier,
    const String& trace, size_t value, const String& units,
    bool important) {
  output += ResultsToString(
      measurement, modifier, trace,
      FormattableToString(static_cast<unsigned int>(value), "d"),
      String(), String(),
      units, important);
}

void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    const String& value, const String& units,
    bool important) {
  PrintResultsImpl(
      measurement, modifier, trace,
      value, String(), String(), units,
      important);
}

void AppendResult(
    String& output,
    const String& measurement, const String& modifier, const String& trace,
    const String& value, const String& units,
    bool important) {
  output += ResultsToString(
      measurement, modifier, trace,
      value, String(), String(), units,
      important);
}

void PrintResultMeanAndError(
    const String& measurement, const String& modifier, const String& trace,
    const String& mean_and_error, const String& units,
    bool important) {
  PrintResultsImpl(
      measurement, modifier, trace, mean_and_error,
      "{", "}", units, important);
}

void AppendResultMeanAndError(
    String& output, const String& measurement, const String& modifier,
    const String& trace, const String& mean_and_error, const String& units,
    bool important) {
  output += ResultsToString(
      measurement, modifier, trace, mean_and_error,
      "{", "}", units, important);
}

void PrintResultList(
    const String& measurement, const String& modifier, const String& trace,
    const String& values, const String& units,
    bool important) {
  PrintResultsImpl(
      measurement, modifier, trace, values,
      "[", "]", units, important);
}

void AppendResultList(
    String& output,
    const String& measurement, const String& modifier,
    const String& trace, const String& values, const String& units,
    bool important) {
  output += ResultsToString(
      measurement, modifier, trace, values,
      "[", "]", units, important);
}

} // namespace perf_test
} // namespace stp
