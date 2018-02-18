// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Build.h"

#include "Base/Containers/Array.h"
#include "Base/Time/Time.h"
#include "Base/Type/ParseInteger.h"

namespace stp {

constexpr Array<StringSpan, 12> MonthArray = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

Time Build::TranslationTime() {
  StringSpan date_str(__DATE__); // "Mmm dd yyyy"
  StringSpan time_str(__TIME__); // "hh:mm:ss"

  Time::Exploded exploded;
  exploded.month = MonthArray.IndexOf(date_str.GetSlice(0, 3));

  StringSpan day_str = date_str.GetSlice(4, 2);
  // First character of "dd" is space if day is lower than ten.
  if (day_str[0] == ' ')
    day_str.RemovePrefix(1);

  parse(day_str, exploded.day_of_month);
  parse(date_str.GetSlice(7, 4), exploded.year);
  parse(time_str.GetSlice(0, 2), exploded.hour);
  parse(time_str.GetSlice(3, 2), exploded.minute);
  parse(time_str.GetSlice(6, 2), exploded.second);
  exploded.day_of_week = -1;

  Time result;
  if (!Time::FromLocalExploded(exploded, &result))
    return Time();
  return result;
}

} // namespace stp
