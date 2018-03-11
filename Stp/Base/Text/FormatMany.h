// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_FORMATMANY_H_
#define STP_BASE_TEXT_FORMATMANY_H_

#include "Base/Containers/Array.h"
#include "Base/Io/TextWriter.h"
#include "Base/Type/Formattable.h"
#include "Base/Util/Tuple.h"

namespace stp {

namespace detail {

class BASE_EXPORT Formatter {
 public:
  virtual void execute(TextWriter& out, const StringSpan& opts) const = 0;
  virtual StringSpan getArgName() const;

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

  void execute(TextWriter& out, const StringSpan& opts) const override {
    format(out, adapted_, opts);
  }

 private:
  const T& adapted_;
};

template<typename T>
class NamedFormatter final : public Formatter {
 public:
  NamedFormatter(StringSpan name, const T& arg)
      : name_(name), adapted_(move(arg)) {}

  void execute(TextWriter& out, const StringSpan& opts) const override {
    format(out, adapted_, opts);
  }

  StringSpan getArgName() const override { return name_; }

 private:
  StringSpan name_;
  const T& adapted_;
};

template<typename T, TEnableIf<!TIsBaseOf<Formatter, T>>* = nullptr>
inline DefaultFormatter<T> buildFormatter(const T& x) {
  return DefaultFormatter<T>(x);
}

template<typename T, TEnableIf<TIsBaseOf<Formatter, T>>* = nullptr>
inline const T& buildFormatter(const T& x) {
  return x;
}

BASE_EXPORT void formatManyImpl(TextWriter& out, StringSpan fmt, Span<Formatter*> args);

} // namespace detail

template<typename... Ts>
inline void formatMany(TextWriter& out, StringSpan fmt, const Ts&... args) {
  auto formatters = makeTuple(detail::buildFormatter(args)...);
  auto pargs = formatters.apply(detail::CreateFormatterArray());
  detail::formatManyImpl(out, fmt, pargs);
}

template<typename T>
inline detail::NamedFormatter<T> formatArg(StringSpan name, const T& value) {
  return detail::NamedFormatter<T>(name, value);
}

} // namespace stp

#endif // STP_BASE_TEXT_FORMATMANY_H_
