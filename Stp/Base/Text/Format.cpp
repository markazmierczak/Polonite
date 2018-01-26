// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Format.h"

#include "Base/Debug/Log.h"
#include "Base/Io/ClipTextWriter.h"
#include "Base/Io/InlineStringWriter.h"
#include "Base/Text/AsciiString.h"
#include "Base/Type/ParseInteger.h"

namespace stp {
namespace detail {

namespace {

class FormatArgId {
 public:
  FormatArgId() = default;

  bool IsName() const { return name_ != nullptr; }
  bool IsIndex() const { return name_ == nullptr && size_or_index_ >= 0; }

  StringSpan AsName() const {
    ASSERT(IsName());
    return StringSpan(name_, size_or_index_);
  }

  int AsIndex() const {
    ASSERT(IsIndex());
    return size_or_index_;
  }

  bool Parse(StringSpan s) {
    // Empty - neither name nor index.
    if (s.IsEmpty())
      return true;

    // First character is digit - treat as index.
    if (IsDigitAscii(s.GetFirst()))
      return TryParse(s, size_or_index_) == ParseIntegerErrorCode::Ok;

    // Otherwise treat as name.
    name_ = s.data();
    size_or_index_ = s.size();
    return true;
  }

 private:
  const char* name_ = nullptr;
  int size_or_index_ = -1;
};

enum class FormatAlign : char {
  None = 0,
  Left = '<',
  Center = '^',
  Right = '>',
};

struct FormatLayout {
  int width = -1;

  FormatAlign align = FormatAlign::Left;

  char fill = ' ';

  bool Parse(StringSpan s);
};

struct FormatReplacement {
  FormatArgId arg_id;
  FormatLayout layout;
  StringSpan options;

  bool NeedsLayout() const { return layout.width >= 0; }

  bool Parse(StringSpan s);

  Formatter* FindFormatter(Span<Formatter*> args, int& implicit_index);
};

static inline FormatAlign ParseAlign(char c) {
  switch (c) {
    case '<':
    case '>':
    case '^':
      return static_cast<FormatAlign>(c);
  }
  return FormatAlign::None;
}

bool FormatLayout::Parse(StringSpan s) {
  if (s.IsEmpty())
    return true;

  // If s[1] is a fill char, s[0] is a pad char and s[2:...] contains the width.
  // Otherwise, if s[0] is a align char, then s[1:...] contains the width.
  // Otherwise, s[0:...] contains the width.
  if (s.size() >= 2) {
    align = ParseAlign(s[1]);
    if (align != FormatAlign::None) {
      fill = s[0];
      s.RemovePrefix(2);
    } else {
      align = ParseAlign(s[0]);
      if (align != FormatAlign::None) {
        s.RemovePrefix(1);
      } else {
        align = FormatAlign::Right;
      }
    }
  }
  return TryParse(s, width) == ParseIntegerErrorCode::Ok;
}

bool FormatReplacement::Parse(StringSpan s) {
  TrimWhitespaceAscii(s);

  int comma = s.IndexOf(',');
  int semicolon = s.IndexOf(':');

  // options specification can also use comma.
  // Detect the case when comma is specified after semicolon only.
  if (semicolon >= 0 && comma > semicolon)
    comma = -1;

  int id_count = comma >= 0 ? comma : (semicolon >= 0 ? semicolon : s.size());

  StringSpan id_spec = s.GetSlice(0, id_count);
  if (!arg_id.Parse(id_spec))
    return false;

  if (comma != -1) {
    int layout_start = comma + 1;
    int layout_end = semicolon >= 0 ? semicolon : s.size();
    StringSpan layout_spec = s.GetSlice(layout_start, layout_end - layout_start);
    if (!layout.Parse(layout_spec))
      return false;
  }
  if (semicolon != -1) {
    options = s.GetSlice(semicolon + 1);
    TrimWhitespaceAscii(options);
  }
  return true;
}

Formatter* FormatReplacement::FindFormatter(
    Span<Formatter*> args,
    int& implicit_index) {
  int index;
  if (arg_id.IsIndex()) {
    index = arg_id.AsIndex();
  } else if (arg_id.IsName()) {
    StringSpan name = arg_id.AsName();
    for (index = 0; index < args.size(); ++index) {
      Formatter* formatter = args[index];
      if (formatter->GetArgName() == name)
        break;
    }
    if (index >= args.size())
      return nullptr;
  } else {
    index = implicit_index + 1;
  }
  implicit_index = index;
  return index < args.size() ? args[index] : nullptr;
}

} // namespace

StringSpan Formatter::GetArgName() const {
  return StringSpan();
}

static void FormatAndLayoutReplacement(
    TextWriter& out,
    const FormatReplacement& replacement,
    Formatter* formatter, const StringSpan& opts) {
  int width = replacement.layout.width;
  ASSERT(width >= 0);

  FormatAlign align = replacement.layout.align;
  char fill = replacement.layout.fill;

  // Write replacement into temporary buffer.
  InlineList<char, 512> buffer;
  InlineStringWriter base_writer(&buffer);
  ClipTextWriter writer(base_writer, width);
  formatter->Execute(writer, opts);

  int pad_length = width - buffer.size();

  // Insert padding on the left side.
  if (pad_length > 0 && align != FormatAlign::Left) {
    if (align == FormatAlign::Right) {
      out.Indent(pad_length, fill);
      pad_length = 0;
    } else {
      ASSERT(align == FormatAlign::Center);
      out.Indent(pad_length / 2, fill);
      pad_length -= pad_length / 2;
    }
  }

  // Forward temporary buffer to the output.
  out.Write(buffer);

  // Insert padding on the right side.
  if (pad_length > 0)
    out.Indent(pad_length, fill);
}

void FormatManyImpl(TextWriter& out, StringSpan fmt, Span<Formatter*> args) {
  // TODO handle double }} correctly
  int implicit_index = -1;
  while (!fmt.IsEmpty()) {
    int brace = fmt.IndexOf('{');
    if (brace < 0) {
      out.Write(fmt);
      break;
    }
    if (fmt[brace + 1] == '{') {
      // handle double braces
      out.Write(fmt.GetSlice(0, brace + 1));
      fmt.RemovePrefix(brace + 2);
    } else { // handle replacement
      out.Write(fmt.GetSlice(0, brace));
      fmt.RemovePrefix(brace + 1);

      // Find replacement boundaries.
      int closing_brace = fmt.IndexOf('}');
      ASSERT(closing_brace >= 0);
      StringSpan rep_string = fmt.GetSlice(0, closing_brace);
      fmt.RemovePrefix(closing_brace + 1);

      FormatReplacement replacement;
      if (!replacement.Parse(rep_string)) {
        LOG(WARN, "invalid replacement at {}", brace);
        // TODO information in exception and remove the log
        throw FormatException();
      }
      // Resolve formatter that will produce input for replacement.
      Formatter* formatter = replacement.FindFormatter(args, implicit_index);
      ASSERT(formatter);

      if (replacement.NeedsLayout())
        FormatAndLayoutReplacement(out, replacement, formatter, replacement.options);
      else
        formatter->Execute(out, replacement.options);
    }
  }
}

} // namespace detail
} // namespace stp
