// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Json/JsonValue.h"

#include "Base/Json/JsonArray.h"
#include "Base/Json/JsonObject.h"
#include "Base/Test/GTest.h"

namespace stp {

TEST(JsonValueTest, Basic) {
  // Test basic object getting/setting
  JsonObject settings;
  StringSpan homepage = "http://google.com";
  ASSERT_FALSE(settings.TryGetWithPath("global.homepage", homepage));
  ASSERT_EQ("http://google.com", homepage);

  ASSERT_FALSE(settings.TryGetWithPath("global"));
  settings.SetWithPath("global", true);
  ASSERT_TRUE(settings.TryGetWithPath("global"));
  settings.SetWithPath("global.homepage", "http://scurvy.com");
  ASSERT_TRUE(settings.TryGetWithPath("global"));
  homepage = "http://google.com";
  ASSERT_TRUE(settings.TryGetObjectWithPath("global"));
  ASSERT_TRUE(settings.TryGetWithPath("global.homepage", homepage));
  ASSERT_EQ("http://scurvy.com", homepage);

  // Test storing an object in an array.
  JsonArray* toolbar_bookmarks = settings.TryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_FALSE(toolbar_bookmarks);

  JsonArray new_toolbar_bookmarks;
  settings.SetWithPath("global.toolbar.bookmarks", Move(new_toolbar_bookmarks));
  toolbar_bookmarks = settings.TryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_TRUE(toolbar_bookmarks);

  JsonObject new_bookmark;
  new_bookmark.SetWithPath("name", "Froogle");
  new_bookmark.SetWithPath("url", "http://froogle.com");
  toolbar_bookmarks->Add(Move(new_bookmark));

  JsonArray* bookmark_array = settings.TryGetArrayWithPath("global.toolbar.bookmarks");
  ASSERT_TRUE(bookmark_array);
  ASSERT_EQ(1, bookmark_array->size());
  JsonObject* bookmark = bookmark_array->TryGetObject(0);
  ASSERT_TRUE(bookmark);
  StringSpan bookmark_name = "Unnamed";
  ASSERT_TRUE(bookmark->TryGetWithPath("name", bookmark_name));
  ASSERT_EQ("Froogle", bookmark_name);
  StringSpan bookmark_url;
  ASSERT_TRUE(bookmark->TryGetWithPath("url", bookmark_url));
  ASSERT_EQ("http://froogle.com", bookmark_url);
}

TEST(JsonValueTest, Array) {
  JsonArray mixed_array;
  mixed_array.Set(0, JsonValue(true));
  mixed_array.Set(1, JsonValue(42));
  mixed_array.Set(2, JsonValue(88.8));
  mixed_array.Set(3, JsonValue("foo"));
  ASSERT_EQ(4, mixed_array.size());

  bool bool_value = false;
  int int_value = 0;
  double double_value = 0.0;
  StringSpan string_value;

  JsonValue* value = mixed_array.TryGet(4);
  ASSERT_FALSE(value);

  ASSERT_FALSE(mixed_array.TryGet(0, int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_array.TryGet(1, bool_value));
  ASSERT_FALSE(bool_value);
  ASSERT_FALSE(mixed_array.TryGet(2, string_value));
  ASSERT_EQ("", string_value);
  ASSERT_FALSE(mixed_array.TryGet(2, int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_array.TryGet(3, bool_value));
  ASSERT_FALSE(bool_value);

  ASSERT_TRUE(mixed_array.TryGet(0, bool_value));
  ASSERT_TRUE(bool_value);
  ASSERT_TRUE(mixed_array.TryGet(1, int_value));
  ASSERT_EQ(42, int_value);
  // implicit conversion from Integer to Double should be possible.
  ASSERT_TRUE(mixed_array.TryGet(1, double_value));
  ASSERT_EQ(42, double_value);
  ASSERT_TRUE(mixed_array.TryGet(2, double_value));
  ASSERT_EQ(88.8, double_value);
  ASSERT_TRUE(mixed_array.TryGet(3, string_value));
  ASSERT_EQ("foo", string_value);

  JsonValue sought_value(42);
  JsonValue not_found_value(false);

  // Attempt to search in the mixed array.
  ASSERT_TRUE(mixed_array.Contains(sought_value));
  ASSERT_FALSE(mixed_array.Contains(not_found_value));
}

} // namespace stp
