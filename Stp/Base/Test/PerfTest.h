// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_PERFTEST_H_
#define STP_BASE_TEST_PERFTEST_H_

#include "Base/Containers/List.h"

namespace stp {
namespace perf_test {

// Prints numerical information to stdout in a controlled format, for
// post-processing. |measurement| is a description of the quantity being
// measured, e.g. "vm_peak"; |modifier| is provided as a convenience and
// will be appended directly to the name of the |measurement|, e.g.
// "_browser"; |trace| is a description of the particular data point, e.g.
// "reference"; |value| is the measured value; and |units| is a description
// of the units of measure, e.g. "bytes". If |important| is true, the output
// line will be specially marked, to notify the post-processor. The strings
// may be empty.  They should not contain any colons (:) or equals signs (=).
// A typical post-processing step would be to produce graphs of the data
// produced for various builds, using the combined |measurement| + |modifier|
// string to specify a particular graph and the |trace| to identify a trace
// (i.e., data series) on that graph.
void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    size_t value, const String& units,
    bool important);
void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    double value, const String& units,
    bool important);

void AppendResult(
    String& output,
    const String& measurement, const String& modifier, const String& trace,
    size_t value, const String& units,
    bool important);

// Like the above version of PrintResult(), but takes a String value
// instead of a size_t.
void PrintResult(
    const String& measurement, const String& modifier, const String& trace,
    const String& value, const String& units,
    bool important);

void AppendResult(
    String& output,
    const String& measurement, const String& modifier, const String& trace,
    const String& value, const String& units,
    bool important);

// Like PrintResult(), but prints a (mean, standard deviation) result pair.
// The |<values>| should be two comma-separated numbers, the mean and
// standard deviation (or other error metric) of the measurement.
void PrintResultMeanAndError(
    const String& measurement, const String& modifier, const String& trace,
    const String& mean_and_error, const String& units,
    bool important);

void AppendResultMeanAndError(
    String& output,
    const String& measurement, const String& modifier, const String& trace,
    const String& mean_and_error, const String& units,
    bool important);

// Like PrintResult(), but prints an entire list of results. The |values|
// will generally be a list of comma-separated numbers. A typical
// post-processing step might produce plots of their mean and standard
// deviation.
void PrintResultList(
    const String& measurement, const String& modifier, const String& trace,
    const String& values, const String& units,
    bool important);

void AppendResultList(
    String& output,
    const String& measurement, const String& modifier, const String& trace,
    const String& values, const String& units,
    bool important);

} // namespace perf_test
} // namespace stp

#endif // STP_BASE_TEST_PERFTEST_H_
