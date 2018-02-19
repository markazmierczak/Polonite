// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonValue.h"

#include "Base/Test/GTest.h"
#include "Json/JsonArray.h"
#include "Json/JsonObject.h"

namespace stp {

TEST(JsonValueTest, Basic) {
  // Test basic object getting/setting
  JsonObject settings;
  StringSpan homepage = "http://google.com";
  ASSERT_FALSE(settings.tryGetWithPath("global.homepage", homepage));
  ASSERT_EQ("http://google.com", homepage);

  ASSERT_FALSE(settings.tryGetWithPath("global"));
  settings.SetWithPath("global", true);
  ASSERT_TRUE(settings.tryGetWithPath("global"));
  settings.SetWithPath("global.homepage", "http://scurvy.com");
  ASSERT_TRUE(settings.tryGetWithPath("global"));
  homepage = "http://google.com";
  ASSERT_TRUE(settings.tryGetObjectWithPath("global"));
  ASSERT_TRUE(settings.tryGetWithPath("global.homepage", homepage));
  ASSERT_EQ("http://scurvy.com", homepage);

  // Test storing an object in an array.
  JsonArray* toolbar_bookmarks = settings.tryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_FALSE(toolbar_bookmarks);

  JsonArray new_toolbar_bookmarks;
  settings.SetWithPath("global.toolbar.bookmarks", move(new_toolbar_bookmarks));
  toolbar_bookmarks = settings.tryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_TRUE(toolbar_bookmarks);

  JsonObject new_bookmark;
  new_bookmark.SetWithPath("name", "Froogle");
  new_bookmark.SetWithPath("url", "http://froogle.com");
  toolbar_bookmarks->add(move(new_bookmark));

  JsonArray* bookmark_array = settings.tryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_TRUE(bookmark_array);
  ASSERT_EQ(1, bookmark_array->size());
  JsonObject* bookmark = bookmark_array->tryGetObject(0);
  ASSERT_TRUE(bookmark);
  StringSpan bookmark_name = "Unnamed";
  ASSERT_TRUE(bookmark->tryGetWithPath("name", bookmark_name));
  ASSERT_EQ("Froogle", bookmark_name);
  StringSpan bookmark_url;
  ASSERT_TRUE(bookmark->tryGetWithPath("url", bookmark_url));
  ASSERT_EQ("http://froogle.com", bookmark_url);
}

TEST(JsonValueTest, Array) {
  JsonArray mixed_array;
  mixed_array.set(0, JsonValue(true));
  mixed_array.set(1, JsonValue(42));
  mixed_array.set(2, JsonValue(88.8));
  mixed_array.set(3, JsonValue("foo"));
  ASSERT_EQ(4, mixed_array.size());

  bool bool_value = false;
  int int_value = 0;
  double double_value = 0.0;
  StringSpan string_value;

  JsonValue* value = mixed_array.tryGet(4);
  ASSERT_FALSE(value);

  ASSERT_FALSE(mixed_array.tryGet(0, int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_array.tryGet(1, bool_value));
  ASSERT_FALSE(bool_value);
  ASSERT_FALSE(mixed_array.tryGet(2, string_value));
  ASSERT_EQ("", string_value);
  ASSERT_FALSE(mixed_array.tryGet(2, int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_array.tryGet(3, bool_value));
  ASSERT_FALSE(bool_value);

  ASSERT_TRUE(mixed_array.tryGet(0, bool_value));
  ASSERT_TRUE(bool_value);
  ASSERT_TRUE(mixed_array.tryGet(1, int_value));
  ASSERT_EQ(42, int_value);
  // implicit conversion from Integer to Double should be possible.
  ASSERT_TRUE(mixed_array.tryGet(1, double_value));
  ASSERT_EQ(42, double_value);
  ASSERT_TRUE(mixed_array.tryGet(2, double_value));
  ASSERT_EQ(88.8, double_value);
  ASSERT_TRUE(mixed_array.tryGet(3, string_value));
  ASSERT_EQ("foo", string_value);

  JsonValue sought_value(42);
  JsonValue not_found_value(false);

  // Attempt to search in the mixed array.
  ASSERT_TRUE(mixed_array.contains(sought_value));
  ASSERT_FALSE(mixed_array.contains(not_found_value));
}

} // namespace stp
