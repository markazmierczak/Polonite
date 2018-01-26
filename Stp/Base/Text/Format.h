// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_FORMAT_H_
#define STP_BASE_TEXT_FORMAT_H_

#include "Base/Containers/Array.h"
#include "Base/Type/Formattable.h"
#include "Base/Util/Tuple.h"

namespace stp {

namespace detail {

class Formatter {
 public:
  virtual void Execute(TextWriter& out, const StringSpan& opts) const = 0;
  virtual StringSpan GetArgName() const;

 protected:
  virtual ~Formatter() {}
};

struct CreateFormatterArray {
  template<typename... Ts>
  auto operator()(Ts&... items) {
    return Array<Formatter*, sizeof...(Ts)> { &items... };
  }
};

template<typename T>
class DefaultFormatter final : public Formatter {
 public:
  explicit DefaultFormatter(const T& adapted) : adapted_(adapted) {}

  void Execute(TextWriter& out, const StringSpan& opts) const override {
    Format(out, adapted_, opts);
  }

 private:
  const T& adapted_;
};

template<typename T>
class NamedFormatter final : public Formatter {
 public:
  NamedFormatter(StringSpan name, const T& arg)
      : name_(name), adapted_(Move(arg)) {}

  void Execute(TextWriter& out, const StringSpan& opts) const override {
    Format(out, adapted_, opts);
  }

  StringSpan GetArgName() const override { return name_; }

 private:
  StringSpan name_;
  const T& adapted_;
};

template<typename T>
using ToFormatMember = decltype(declval<const T&>().ToFormat(
    declval<TextWriter&>(), declval<const StringSpan&>()));

template<typename T, TEnableIf<!TIsBaseOf<Formatter, T>>* = nullptr>
inline DefaultFormatter<T> BuildFormatter(const T& x) {
  return DefaultFormatter<T>(x);
}

template<typename T, TEnableIf<TIsBaseOf<Formatter, T>>* = nullptr>
inline const T& BuildFormatter(const T& x) {
  return x;
}

BASE_EXPORT void FormatManyImpl(TextWriter& out, StringSpan fmt, Span<Formatter*> args);

} // namespace detail

template<typename... Ts>
inline void FormatMany(TextWriter& out, StringSpan fmt, const Ts&... args) {
  auto formatters = MakeTuple(detail::BuildFormatter(args)...);
  auto pargs = formatters.Apply(detail::CreateFormatterArray());
  detail::FormatManyImpl(out, fmt, pargs);
}

template<typename T>
inline detail::NamedFormatter<T> FormatArg(StringSpan name, const T& value) {
  return detail::NamedFormatter<T>(name, value);
}

template<typename T, TEnableIf<THasDetected<detail::ToFormatMember, T>>* = nullptr>
inline void Format(TextWriter& out, const T& s, const StringSpan& opts) {
  s.ToFormat(out, opts);
}

template<typename... Ts>
inline void AssertFail(
    const char* file, int line, const char* expr,
    StringSpan fmt, const Ts&... args) {
  TextWriter& out = AssertPrint(file, line, expr);
  FormatMany(out, fmt, args...);
  AssertWrapUp(out);
}

template<typename T, typename... TArgs>
inline List<T> StringTmplFormatMany(StringSpan fmt, const TArgs&... args) {
  List<T> result;
  StringTmplWriter<T> writer(&result);
  FormatMany(writer, fmt, args...);
  return result;
}

template<typename... TArgs>
inline String StringFormatMany(StringSpan fmt, const TArgs&... args) {
  return StringTmplFormatMany<char>(fmt, args...);
}

template<typename T, typename TValue>
inline List<T> FormattableToStringTmpl(const TValue& value, const StringSpan& opts = StringSpan()) {
  List<T> result;
  StringTmplWriter<T> writer(&result);
  Format(writer, value, opts);
  return result;
}

template<typename T>
inline String FormattableToString(const T& value, const StringSpan& opts = StringSpan()) {
  return FormattableToStringTmpl<char>(value, opts);
}
template<typename T>
inline String16 FormattableToString16(const T& value, const StringSpan& opts) {
  return FormattableToStringTmpl<char16_t>(value, opts);
}

} // namespace stp

#endif // STP_BASE_TEXT_FORMAT_H_
