// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/Time.h"

#include "Base/Test/GTest.h"
#include "Base/Type/FormattableToString.h"

namespace stp {

namespace {

TEST(TimeTestOutOfBounds, FromExplodedOutOfBoundsTime) {
  // FromUTCExploded must set time to Time(0) and failure, if the day is set to
  // 31 on a 28-30 day month. Test |exploded| returns Time(0) on 31st of
  // February and 31st of April. New implementation handles this.

  const struct DateTestData {
    Time::Exploded explode;
    bool is_valid;
  } DateTestData[] = {
    // 31st of February
    {{2016, 2, 0, 31, 12, 30, 0, 0}, true},
    // 31st of April
    {{2016, 4, 0, 31, 8, 43, 0, 0}, true},
    // Negative month
    {{2016, -5, 0, 2, 4, 10, 0, 0}, false},
    // Negative date of month
    {{2016, 6, 0, -15, 2, 50, 0, 0}, false},
    // Negative hours
    {{2016, 7, 0, 10, -11, 29, 0, 0}, false},
    // Negative minutes
    {{2016, 3, 0, 14, 10, -29, 0, 0}, false},
    // Negative seconds
    {{2016, 10, 0, 25, 7, 47, -30, 0}, false},
    // Negative milliseconds
    {{2016, 10, 0, 25, 7, 47, 20, -500}, false},
    // Hours are too large
    {{2016, 7, 0, 10, 26, 29, 0, 0}, false},
    // Minutes are too large
    {{2016, 3, 0, 14, 10, 78, 0, 0}, false},
    // Seconds are too large
    {{2016, 10, 0, 25, 7, 47, 234, 0}, false},
    // Milliseconds are too large
    {{2016, 10, 0, 25, 6, 31, 23, 1643}, false},
  };

  for (const auto& test : DateTestData) {
    EXPECT_EQ(test.explode.HasValidValues(), test.is_valid);

    Time result;
    EXPECT_FALSE(Time::FromUTCExploded(test.explode, &result));
    EXPECT_TRUE(result.isNull());
    EXPECT_FALSE(Time::FromLocalExploded(test.explode, &result));
    EXPECT_TRUE(result.isNull());
  }
}

// Specialized test fixture allowing time strings without timezones to be
// tested by comparing them to a known time in the local zone.
class TimeTest : public testing::Test {
 protected:
  void SetUp() override {
    // Use mktime to get a time_t, and turn it into a PRTime by converting
    // seconds to microseconds.  Use 15th Oct 2007 12:45:00 local.  This
    // must be a time guaranteed to be outside of a DST fallback hour in
    // any timezone.
    struct tm local_comparison_tm = {
      0,            // second
      45,           // minute
      12,           // hour
      15,           // day of month
      10 - 1,       // month
      2007 - 1900,  // year
      0,            // day of week (ignored, output only)
      0,            // day of year (ignored, output only)
      -1            // DST in effect, -1 tells mktime to figure it out
    };

    time_t converted_time = mktime(&local_comparison_tm);
    ASSERT_GT(converted_time, 0);
    comparison_time_local_ = Time::FromTimeT(converted_time);

    // time_t representation of 15th Oct 2007 12:45:00 PDT
    comparison_time_pdt_ = Time::FromTimeT(1192477500);
  }

  Time comparison_time_local_;
  Time comparison_time_pdt_;
};

// Test conversions to/from time_t and exploding/unexploding.
TEST_F(TimeTest, TimeT) {
  // C library time and exploded time.
  time_t now_t_1 = time(NULL);
  struct tm tms;
  #if OS(WIN)
  localtime_s(&tms, &now_t_1);
  #elif OS(POSIX)
  localtime_r(&now_t_1, &tms);
  #endif

  // Convert to ours.
  Time our_time_1 = Time::FromTimeT(now_t_1);
  Time::Exploded exploded;
  our_time_1.LocalExplode(&exploded);

  // This will test both our exploding and our time_t -> Time conversion.
  EXPECT_EQ(tms.tm_year + 1900, exploded.year);
  EXPECT_EQ(tms.tm_mon + 1, exploded.month);
  EXPECT_EQ(tms.tm_mday, exploded.day_of_month);
  EXPECT_EQ(tms.tm_hour, exploded.hour);
  EXPECT_EQ(tms.tm_min, exploded.minute);
  EXPECT_EQ(tms.tm_sec, exploded.second);

  // Convert exploded back to the time struct.
  Time our_time_2;
  EXPECT_TRUE(Time::FromLocalExploded(exploded, &our_time_2));
  EXPECT_TRUE(our_time_1 == our_time_2);

  time_t now_t_2 = our_time_2.ToTimeT();
  EXPECT_EQ(now_t_1, now_t_2);

  EXPECT_EQ(10, Time().FromTimeT(10).ToTimeT());
  EXPECT_EQ(10.0, Time().FromTimeT(10).ToDoubleT());

  // Conversions of 0 should stay 0.
  EXPECT_EQ(0, Time().ToTimeT());
  EXPECT_EQ(0, Time::FromTimeT(0).ToInternalValue());
}

// Test conversions to/from javascript time.
TEST_F(TimeTest, JsTime) {
  Time epoch = Time::FromJSTime(0.0);
  EXPECT_EQ(epoch, Time::UnixEpoch());
  Time t = Time::FromJSTime(700000.3);
  EXPECT_EQ(700.0003, t.ToDoubleT());
  t = Time::FromDoubleT(800.73);
  EXPECT_EQ(800730.0, t.ToJSTime());
}

#if OS(POSIX)
TEST_F(TimeTest, FromTimeVal) {
  Time now = Time::Now();
  Time also_now = Time::FromTimeVal(now.ToTimeVal());
  EXPECT_EQ(now, also_now);
}
#endif // OS(POSIX)

TEST_F(TimeTest, FromExplodedWithMilliseconds) {
  // Some platform implementations of FromExploded are liable to drop
  // milliseconds if we aren't careful.
  Time now = Time::Now();
  Time::Exploded exploded1 = {0};
  now.UTCExplode(&exploded1);
  exploded1.millisecond = 500;
  Time time;
  EXPECT_TRUE(Time::FromUTCExploded(exploded1, &time));
  Time::Exploded exploded2 = {0};
  time.UTCExplode(&exploded2);
  EXPECT_EQ(exploded1.millisecond, exploded2.millisecond);
}

TEST_F(TimeTest, ZeroIsSymmetric) {
  Time zero_time(Time::FromTimeT(0));
  EXPECT_EQ(0, zero_time.ToTimeT());

  EXPECT_EQ(0.0, zero_time.ToDoubleT());
}

TEST_F(TimeTest, LocalExplode) {
  Time a = Time::Now();
  Time::Exploded exploded;
  a.LocalExplode(&exploded);

  Time b;
  EXPECT_TRUE(Time::FromLocalExploded(exploded, &b));

  // The exploded structure doesn't have microseconds, and on Mac & Linux, the
  // internal OS conversion uses seconds, which will cause truncation. So we
  // can only make sure that the delta is within one second.
  EXPECT_TRUE((a - b) < TimeDelta::FromSeconds(1));
}

TEST_F(TimeTest, UTCExplode) {
  Time a = Time::Now();
  Time::Exploded exploded;
  a.UTCExplode(&exploded);

  Time b;
  EXPECT_TRUE(Time::FromUTCExploded(exploded, &b));
  EXPECT_TRUE((a - b) < TimeDelta::FromSeconds(1));
}

TEST_F(TimeTest, LocalMidnight) {
  Time::Exploded exploded;
  Time::Now().LocalMidnight().LocalExplode(&exploded);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(0, exploded.second);
  EXPECT_EQ(0, exploded.millisecond);
}

TEST_F(TimeTest, ParseTimeTest1) {
  time_t current_time = 0;
  time(&current_time);

  const int BUFFER_SIZE = 64;
  struct tm local_time = {0};
  char time_buf[BUFFER_SIZE] = {0};
  #if OS(WIN)
  localtime_s(&local_time, &current_time);
  asctime_s(time_buf, isizeofArray(time_buf), &local_time);
  #elif OS(POSIX)
  localtime_r(&current_time, &local_time);
  asctime_r(&local_time, time_buf);
  #endif

  Time parsed_time;
  EXPECT_TRUE(Time::FromString(time_buf, &parsed_time));
  EXPECT_EQ(current_time, parsed_time.ToTimeT());
}

TEST_F(TimeTest, DayOfWeekSunday) {
  Time time;
  EXPECT_TRUE(Time::FromString("Sun, 06 May 2012 12:00:00 GMT", &time));
  Time::Exploded exploded;
  time.UTCExplode(&exploded);
  EXPECT_EQ(0, exploded.day_of_week);
}

TEST_F(TimeTest, DayOfWeekWednesday) {
  Time time;
  EXPECT_TRUE(Time::FromString("Wed, 09 May 2012 12:00:00 GMT", &time));
  Time::Exploded exploded;
  time.UTCExplode(&exploded);
  EXPECT_EQ(3, exploded.day_of_week);
}

TEST_F(TimeTest, DayOfWeekSaturday) {
  Time time;
  EXPECT_TRUE(Time::FromString("Sat, 12 May 2012 12:00:00 GMT", &time));
  Time::Exploded exploded;
  time.UTCExplode(&exploded);
  EXPECT_EQ(6, exploded.day_of_week);
}

TEST_F(TimeTest, ParseTimeTest2) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("Mon, 15 Oct 2007 19:45:00 GMT", &parsed_time));
  EXPECT_EQ(comparison_time_pdt_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest3) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("15 Oct 07 12:45:00", &parsed_time));
  EXPECT_EQ(comparison_time_local_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest4) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("15 Oct 07 19:45 GMT", &parsed_time));
  EXPECT_EQ(comparison_time_pdt_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest5) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("Mon Oct 15 12:45 PDT 2007", &parsed_time));
  EXPECT_EQ(comparison_time_pdt_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest6) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("Monday, Oct 15, 2007 12:45 PM", &parsed_time));
  EXPECT_EQ(comparison_time_local_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest7) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("10/15/07 12:45:00 PM", &parsed_time));
  EXPECT_EQ(comparison_time_local_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest8) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("15-OCT-2007 12:45pm", &parsed_time));
  EXPECT_EQ(comparison_time_local_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest9) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("16 Oct 2007 4:45-JST (Tuesday)", &parsed_time));
  EXPECT_EQ(comparison_time_pdt_, parsed_time);
}

TEST_F(TimeTest, ParseTimeTest10) {
  Time parsed_time;
  EXPECT_TRUE(Time::FromString("15/10/07 12:45", &parsed_time));
  EXPECT_EQ(parsed_time, comparison_time_local_);
}

// Test some of edge cases around epoch, etc.
TEST_F(TimeTest, ParseTimeTestEpoch0) {
  Time parsed_time;

  // time_t == epoch == 0
  EXPECT_TRUE(Time::FromString("Thu Jan 01 01:00:00 +0100 1970", &parsed_time));
  EXPECT_EQ(0, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Thu Jan 01 00:00:00 GMT 1970", &parsed_time));
  EXPECT_EQ(0, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEpoch1) {
  Time parsed_time;

  // time_t == 1 second after epoch == 1
  EXPECT_TRUE(Time::FromString("Thu Jan 01 01:00:01 +0100 1970", &parsed_time));
  EXPECT_EQ(1, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Thu Jan 01 00:00:01 GMT 1970", &parsed_time));
  EXPECT_EQ(1, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEpoch2) {
  Time parsed_time;

  // time_t == 2 seconds after epoch == 2
  EXPECT_TRUE(Time::FromString("Thu Jan 01 01:00:02 +0100 1970", &parsed_time));
  EXPECT_EQ(2, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Thu Jan 01 00:00:02 GMT 1970", &parsed_time));
  EXPECT_EQ(2, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEpochNeg1) {
  Time parsed_time;

  // time_t == 1 second before epoch == -1
  EXPECT_TRUE(Time::FromString("Thu Jan 01 00:59:59 +0100 1970", &parsed_time));
  EXPECT_EQ(-1, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Wed Dec 31 23:59:59 GMT 1969", &parsed_time));
  EXPECT_EQ(-1, parsed_time.ToTimeT());
}

// If time_t is 32 bits, a date after year 2038 will overflow time_t and
// cause timegm() to return -1.  The parsed time should not be 1 second
// before epoch.
TEST_F(TimeTest, ParseTimeTestEpochNotNeg1) {
  Time parsed_time;

  EXPECT_TRUE(Time::FromString("Wed Dec 31 23:59:59 GMT 2100", &parsed_time));
  EXPECT_NE(-1, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEpochNeg2) {
  Time parsed_time;

  // time_t == 2 seconds before epoch == -2
  EXPECT_TRUE(Time::FromString("Thu Jan 01 00:59:58 +0100 1970", &parsed_time));
  EXPECT_EQ(-2, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Wed Dec 31 23:59:58 GMT 1969", &parsed_time));
  EXPECT_EQ(-2, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEpoch1960) {
  Time parsed_time;

  // time_t before Epoch, in 1960
  EXPECT_TRUE(Time::FromString("Wed Jun 29 19:40:01 +0100 1960", &parsed_time));
  EXPECT_EQ(-299999999, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Wed Jun 29 18:40:01 GMT 1960", &parsed_time));
  EXPECT_EQ(-299999999, parsed_time.ToTimeT());
  EXPECT_TRUE(Time::FromString("Wed Jun 29 17:40:01 GMT 1960", &parsed_time));
  EXPECT_EQ(-300003599, parsed_time.ToTimeT());
}

TEST_F(TimeTest, ParseTimeTestEmpty) {
  Time parsed_time;
  EXPECT_FALSE(Time::FromString("", &parsed_time));
}

TEST_F(TimeTest, ParseTimeTestInvalidString) {
  Time parsed_time;
  EXPECT_FALSE(Time::FromString("Monday morning 2000", &parsed_time));
}

TEST_F(TimeTest, ExplodeBeforeUnixEpoch) {
  static const int kUnixEpochYear = 1970;  // In case this changes (ha!).
  Time t;
  Time::Exploded exploded;

  t = Time::UnixEpoch() - TimeDelta::FromMicroseconds(1);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1969-12-31 23:59:59 999 milliseconds (and 999 microseconds).
  EXPECT_EQ(kUnixEpochYear - 1, exploded.year);
  EXPECT_EQ(12, exploded.month);
  EXPECT_EQ(31, exploded.day_of_month);
  EXPECT_EQ(23, exploded.hour);
  EXPECT_EQ(59, exploded.minute);
  EXPECT_EQ(59, exploded.second);
  EXPECT_EQ(999, exploded.millisecond);

  t = Time::UnixEpoch() - TimeDelta::FromMicroseconds(1000);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1969-12-31 23:59:59 999 milliseconds.
  EXPECT_EQ(kUnixEpochYear - 1, exploded.year);
  EXPECT_EQ(12, exploded.month);
  EXPECT_EQ(31, exploded.day_of_month);
  EXPECT_EQ(23, exploded.hour);
  EXPECT_EQ(59, exploded.minute);
  EXPECT_EQ(59, exploded.second);
  EXPECT_EQ(999, exploded.millisecond);

  t = Time::UnixEpoch() - TimeDelta::FromMicroseconds(1001);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1969-12-31 23:59:59 998 milliseconds (and 999 microseconds).
  EXPECT_EQ(kUnixEpochYear - 1, exploded.year);
  EXPECT_EQ(12, exploded.month);
  EXPECT_EQ(31, exploded.day_of_month);
  EXPECT_EQ(23, exploded.hour);
  EXPECT_EQ(59, exploded.minute);
  EXPECT_EQ(59, exploded.second);
  EXPECT_EQ(998, exploded.millisecond);

  t = Time::UnixEpoch() - TimeDelta::FromMilliseconds(1000);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1969-12-31 23:59:59.
  EXPECT_EQ(kUnixEpochYear - 1, exploded.year);
  EXPECT_EQ(12, exploded.month);
  EXPECT_EQ(31, exploded.day_of_month);
  EXPECT_EQ(23, exploded.hour);
  EXPECT_EQ(59, exploded.minute);
  EXPECT_EQ(59, exploded.second);
  EXPECT_EQ(0, exploded.millisecond);

  t = Time::UnixEpoch() - TimeDelta::FromMilliseconds(1001);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1969-12-31 23:59:58 999 milliseconds.
  EXPECT_EQ(kUnixEpochYear - 1, exploded.year);
  EXPECT_EQ(12, exploded.month);
  EXPECT_EQ(31, exploded.day_of_month);
  EXPECT_EQ(23, exploded.hour);
  EXPECT_EQ(59, exploded.minute);
  EXPECT_EQ(58, exploded.second);
  EXPECT_EQ(999, exploded.millisecond);

  // Make sure we still handle at/after Unix epoch correctly.
  t = Time::UnixEpoch();
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1970-12-31 00:00:00 0 milliseconds.
  EXPECT_EQ(kUnixEpochYear, exploded.year);
  EXPECT_EQ(1, exploded.month);
  EXPECT_EQ(1, exploded.day_of_month);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(0, exploded.second);
  EXPECT_EQ(0, exploded.millisecond);

  t = Time::UnixEpoch() + TimeDelta::FromMicroseconds(1);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1970-01-01 00:00:00 0 milliseconds (and 1 microsecond).
  EXPECT_EQ(kUnixEpochYear, exploded.year);
  EXPECT_EQ(1, exploded.month);
  EXPECT_EQ(1, exploded.day_of_month);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(0, exploded.second);
  EXPECT_EQ(0, exploded.millisecond);

  t = Time::UnixEpoch() + TimeDelta::FromMicroseconds(1000);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1970-01-01 00:00:00 1 millisecond.
  EXPECT_EQ(kUnixEpochYear, exploded.year);
  EXPECT_EQ(1, exploded.month);
  EXPECT_EQ(1, exploded.day_of_month);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(0, exploded.second);
  EXPECT_EQ(1, exploded.millisecond);

  t = Time::UnixEpoch() + TimeDelta::FromMilliseconds(1000);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1970-01-01 00:00:01.
  EXPECT_EQ(kUnixEpochYear, exploded.year);
  EXPECT_EQ(1, exploded.month);
  EXPECT_EQ(1, exploded.day_of_month);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(1, exploded.second);
  EXPECT_EQ(0, exploded.millisecond);

  t = Time::UnixEpoch() + TimeDelta::FromMilliseconds(1001);
  t.UTCExplode(&exploded);
  EXPECT_TRUE(exploded.HasValidValues());
  // Should be 1970-01-01 00:00:01 1 millisecond.
  EXPECT_EQ(kUnixEpochYear, exploded.year);
  EXPECT_EQ(1, exploded.month);
  EXPECT_EQ(1, exploded.day_of_month);
  EXPECT_EQ(0, exploded.hour);
  EXPECT_EQ(0, exploded.minute);
  EXPECT_EQ(1, exploded.second);
  EXPECT_EQ(1, exploded.millisecond);
}

#if OS(DARWIN)
TEST_F(TimeTest, TimeTOverflow) {
  Time t = Time::FromInternalValue(Limits<int64_t>::Max - 1);
  EXPECT_FALSE(t.IsMax());
  EXPECT_EQ(Limits<time_t>::Max, t.ToTimeT());
}
#endif

#if OS(ANDROID)
TEST_F(TimeTest, FromLocalExplodedCrashOnAndroid) {
  // This crashed inside Time:: FromLocalExploded() on Android 4.1.2.
  Time::Exploded midnight = {2013,  // year
                             10,    // month
                             0,     // day_of_week
                             13,    // day_of_month
                             0,     // hour
                             0,     // minute
                             0,     // second
  };
  // The string passed to putenv() must be a char* and the documentation states
  // that it 'becomes part of the environment', so use a static buffer.
  static char buffer[] = "TZ=America/Santiago";
  putenv(buffer);
  tzset();
  Time t;
  EXPECT_TRUE(Time::FromLocalExploded(midnight, &t));
  EXPECT_EQ(1381633200, t.ToTimeT());
}
#endif // OS(ANDROID)

// Our internal time format is serialized in things like databases, so it's
// important that it's consistent across all our platforms.  We use the 1970
// Unix epoch as the internal format across all platforms.
TEST_F(TimeTest, UnixEpoch) {
  Time::Exploded exploded;
  exploded.year = 1970;
  exploded.month = 1;
  exploded.day_of_week = 0;  // Should be unusued.
  exploded.day_of_month = 1;
  exploded.hour = 0;
  exploded.minute = 0;
  exploded.second = 0;
  exploded.millisecond = 0;
  Time t;
  EXPECT_TRUE(Time::FromUTCExploded(exploded, &t));
  EXPECT_EQ(INT64_C(0), t.ToInternalValue());
}

TEST(TimeLogging, FrameworkBirthdate) {
  Time birthdate;
  ASSERT_TRUE(Time::FromString("Tue, 01 Jan 2017 10:20:30 GMT", &birthdate));
  EXPECT_EQ(StringSpan("2017-01-01 10:20:30.000 UTC"), FormattableToString(birthdate));
}

} // namespace

} // namespace stp
