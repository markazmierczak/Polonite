// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/FormatMany.h"

#include "Base/Debug/Log.h"
#include "Base/Io/ClipTextWriter.h"
#include "Base/Io/InlineStringWriter.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/ParseInteger.h"

namespace stp {
namespace detail {

namespace {

class FormatArgId {
 public:
  FormatArgId() = default;

  bool isName() const { return name_ != nullptr; }
  bool isIndex() const { return name_ == nullptr && size_or_index_ >= 0; }

  StringSpan asName() const {
    ASSERT(isName());
    return StringSpan(name_, size_or_index_);
  }

  int asIndex() const {
    ASSERT(isIndex());
    return size_or_index_;
  }

  bool parse(StringSpan s) {
    // Empty - neither name nor index.
    if (s.isEmpty())
      return true;

    // First character is digit - treat as index.
    if (isDigitAscii(s.getFirst()))
      return tryParse(s, size_or_index_) == ParseIntegerErrorCode::Ok;

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

  bool parse(StringSpan s);
};

struct FormatReplacement {
  FormatArgId arg_id;
  FormatLayout layout;
  StringSpan options;

  bool needsLayout() const { return layout.width >= 0; }

  bool parse(StringSpan s);

  Formatter* findFormatter(Span<Formatter*> args, int& implicit_index);
};

static inline FormatAlign parseAlign(char c) {
  switch (c) {
    case '<':
    case '>':
    case '^':
      return static_cast<FormatAlign>(c);
  }
  return FormatAlign::None;
}

bool FormatLayout::parse(StringSpan s) {
  if (s.isEmpty())
    return true;

  // If s[1] is a fill char, s[0] is a pad char and s[2:...] contains the width.
  // Otherwise, if s[0] is a align char, then s[1:...] contains the width.
  // Otherwise, s[0:...] contains the width.
  if (s.size() >= 2) {
    align = parseAlign(s[1]);
    if (align != FormatAlign::None) {
      fill = s[0];
      s.removePrefix(2);
    } else {
      align = parseAlign(s[0]);
      if (align != FormatAlign::None) {
        s.removePrefix(1);
      } else {
        align = FormatAlign::Right;
      }
    }
  }
  return tryParse(s, width) == ParseIntegerErrorCode::Ok;
}

bool FormatReplacement::parse(StringSpan s) {
  int comma = s.indexOf(',');
  int semicolon = s.indexOf(':');

  // options specification can also use comma.
  // Detect the case when comma is specified after semicolon only.
  if (semicolon >= 0 && comma > semicolon)
    comma = -1;

  int id_count = comma >= 0 ? comma : (semicolon >= 0 ? semicolon : s.size());

  StringSpan id_spec = s.getSlice(0, id_count);
  if (!arg_id.parse(id_spec))
    return false;

  if (comma != -1) {
    int layout_start = comma + 1;
    int layout_end = semicolon >= 0 ? semicolon : s.size();
    StringSpan layout_spec = s.getSlice(layout_start, layout_end - layout_start);
    if (!layout.parse(layout_spec))
      return false;
  }
  if (semicolon != -1) {
    options = s.getSlice(semicolon + 1);
  }
  return true;
}

Formatter* FormatReplacement::findFormatter(Span<Formatter*> args, int& implicit_index) {
  int index;
  if (arg_id.isIndex()) {
    index = arg_id.asIndex();
  } else if (arg_id.isName()) {
    StringSpan name = arg_id.asName();
    for (index = 0; index < args.size(); ++index) {
      Formatter* formatter = args[index];
      if (formatter->getArgName() == name)
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

StringSpan Formatter::getArgName() const {
  return StringSpan();
}

static void formatAndLayoutReplacement(
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
  ClipTextWriter writer(&base_writer, width);
  formatter->execute(writer, opts);

  int pad_length = width - buffer.size();

  // Insert padding on the left side.
  if (pad_length > 0 && align != FormatAlign::Left) {
    if (align == FormatAlign::Right) {
      out.indent(pad_length, fill);
      pad_length = 0;
    } else {
      ASSERT(align == FormatAlign::Center);
      out.indent(pad_length / 2, fill);
      pad_length -= pad_length / 2;
    }
  }

  // Forward temporary buffer to the output.
  out << buffer;

  // Insert padding on the right side.
  if (pad_length > 0)
    out.indent(pad_length, fill);
}

void formatManyImpl(TextWriter& out, StringSpan fmt, Span<Formatter*> args) {
  // TODO handle double }} correctly
  int implicit_index = -1;
  while (!fmt.isEmpty()) {
    int brace = fmt.indexOf('{');
    if (brace < 0) {
      out << fmt;
      break;
    }
    if (fmt[brace + 1] == '{') {
      // handle double braces
      out << fmt.getSlice(0, brace + 1);
      fmt.removePrefix(brace + 2);
    } else { // handle replacement
      out << fmt.getSlice(0, brace);
      fmt.removePrefix(brace + 1);

      // Find replacement boundaries.
      int closing_brace = fmt.indexOf('}');
      ASSERT(closing_brace >= 0);
      StringSpan rep_string = fmt.getSlice(0, closing_brace);
      fmt.removePrefix(closing_brace + 1);

      FormatReplacement replacement;
      if (!replacement.parse(rep_string)) {
        LOG(WARN, "invalid replacement at {}", brace);
        // TODO information in exception and remove the log
        throw FormatException();
      }
      // Resolve formatter that will produce input for replacement.
      Formatter* formatter = replacement.findFormatter(args, implicit_index);
      ASSERT(formatter);

      if (replacement.needsLayout())
        formatAndLayoutReplacement(out, replacement, formatter, replacement.options);
      else
        formatter->execute(out, replacement.options);
    }
  }
}

} // namespace detail
} // namespace stp
