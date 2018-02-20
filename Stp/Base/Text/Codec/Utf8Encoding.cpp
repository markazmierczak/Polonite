// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Codec/Utf8Encoding.h"

#include "Base/Containers/SpanAlgo.h"
#include "Base/Math/Alignment.h"
#include "Base/Text/Utf.h"

namespace stp {
namespace detail {
/*
namespace {

inline char32_t DecodeTrails(char32_t c) { return c; }

template<typename T, typename... TArgs>
inline char32_t DecodeTrails(char32_t c, T trail, TArgs... trails) {
  ASSERT(Utf8::IsEncodedTrail(trail));
  return DecodeTrails((c << 6) | (trail & 0x3F), trails...);
}

void ConvertLongCode(const byte_t* iptr, int& ii, char* optr, int& oi, bool& saw_error) {
  byte_t head = iptr[ii];
  unsigned error = ~head & 0x40;

  byte_t trail0 = iptr[ii + 1];
  error |= (trail0 & 0xC0) ^ 0x80;

  if (head & 0x20) {
    byte_t trail1 = iptr[ii + 2];
    error |= (trail1 & 0xC0) ^ 0x80;

    char32_t bits = ((head & 0x0F) << 6) | (trail0 & 0x3F);

    if (head & 0x10) {
      // 4 byte sequence
      if (!bits || bits > 0x10)
        error |= 1;
      byte_t trail2 = iptr[ii + 3];
      error |= (trail2 & 0xC0) ^ 0x80;
      if (!error) {
        optr[oi++] = head;
        optr[oi++] = trail0;
        optr[oi++] = trail1;
        optr[oi++] = trail2;
        ii += 4;
      }
    } else {
      // 3 byte sequence
      if ((bits & (0x1F << 5)) == 0 ||
          (bits & (0xF8000 >> 6)) == (0xD800 >> 6)) {
        error |= 1;
      }
      if (!error) {
        optr[oi++] = head;
        optr[oi++] = trail0;
        optr[oi++] = trail1;
        ii += 3;
      }
    }
  } else {
    // 2 byte sequence
    if (!error) {
      optr[oi++] = head;
      optr[oi++] = trail0;
      ii += 2;
    }
  }
  if (UNLIKELY(error)) {
    optr[oi++] = Unicode::FallbackUnit<char>;
    ++ii;
    saw_error = true;
  }
}

void ConvertLongCode(const byte_t* iptr, int& ii, char16_t* optr, int& oi, bool& saw_error) {
  byte_t head = iptr[ii];
  unsigned error = ~head & 0x40;

  byte_t trail0 = iptr[ii + 1];
  error |= (trail0 & 0xC0) ^ 0x80;

  if (head & 0x20) {
    byte_t trail1 = iptr[ii + 2];
    error |= (trail1 & 0xC0) ^ 0x80;

    char32_t bits = ((head & 0x0F) << 6) | (trail0 & 0x3F);

    if (head & 0x10) {
      // 4 byte sequence
      if (!bits || bits > 0x10)
        error |= 1;
      byte_t trail2 = iptr[ii + 3];
      error |= (trail2 & 0xC0) ^ 0x80;
      if (!error) {
        char32_t lc = DecodeTrails(bits, trail1, trail2);
        oi += EncodeUtf(optr + oi, lc);
        ii += 4;
      }
    } else {
      // 3 byte sequence
      if ((bits & (0x1F << 5)) == 0 ||
          (bits & (0xF8000 >> 6)) == (0xD800 >> 6)) {
        error |= 1;
      }
      if (!error) {
        optr[oi++] = DecodeTrails(bits, trail1);
        ii += 3;
      }
    }
  } else {
    // 2 byte sequence
    head &= 0x1F;
    if (head <= 1)
      error |= 1;
    if (!error) {
      optr[oi++] = DecodeTrails(head, trail0);
      ii += 2;
    }
  }
  if (UNLIKELY(error)) {
    optr[oi++] = Unicode::FallbackUnit<char16_t>;
    ++ii;
    saw_error = true;
  }
}

class Utf8ReaderState {
 public:
  explicit Utf8ReaderState(const byte_t* bytes) : bytes_(const_cast<byte_t*>(bytes)) {}

  bool MaybeFeed(const byte_t* iptr, int& ii, const int isize) {
    if (!NeedsFlush())
      return false;
    int count_now = GetLength() - 1;
    int count_required = Utf8::TrailLengths[bytes_[0]];
    while (ii < isize && count_now < count_required) {
      if (!Utf8::IsEncodedTrail(iptr[ii]))
        return true;
      bytes_[count_now++] = iptr[ii++];
    }
    return count_now == count_required;
  }

  bool Feed(const byte_t* iptr, int& ii, const int isize) {
    ASSERT(!NeedsFlush());
    ASSERT(ii < isize);
    ASSERT(iptr[ii] >= 0x80);

    bytes_[0] = iptr[ii++];
    int count_now = 0;
    int count_required = Utf8::TrailLengths[bytes_[0]];
    while (ii < isize && count_now < count_required) {
      if (!Utf8::IsEncodedTrail(iptr[ii]))
        return true;
      bytes_[count_now++] = iptr[ii++];
    }
    return count_now == count_required;
  }

  template<typename T>
  void Write(T* optr, int& oi, bool& saw_error) {
    ASSERT(NeedsFlush());
    int ii = 0;
    ConvertLongCode(bytes_, ii, optr, oi, saw_error);
    for (; ii < 4 && bytes_[ii] == 0; ++ii)
      optr[oi++] = Unicode::FallbackUnit<T>;
    Fill(GetSpan(), 0);
  }

  bool NeedsFlush() const { return bytes_[0] != 0; }
  int CountChars() const { return NeedsFlush() ? 4 : 0; }

  int GetLength() { return GetSpan().indexOf(0); }
  MutableSpan<byte_t> GetSpan() { return MutableSpan<byte_t>(bytes_, 4); }

 private:
  byte_t* const bytes_;
};

template<typename T>
inline int DecodeTmpl(Context& context, BufferSpan input, MutableSpan<T> output, bool flush) {
  Utf8ReaderState state(context.state.bytes);

  const int isize = input.size();
  auto* iptr = static_cast<const byte_t*>(input.data());
  int ii = 0;
  auto* optr = output.data();
  int oi = 0;
  bool saw_error = false;

  if (state.MaybeFeed(iptr, ii, isize))
    state.Write(optr, oi, saw_error);

  using MachineWord = uintptr_t;
  constexpr int WordIncrement = isizeof(MachineWord);

  int isize_fast = isize - WordIncrement;
  while (ii < isize_fast) {
    byte_t head = iptr[ii];
    if (head < 0x80) {
      // ASCII
      if (IsAlignedTo(iptr, WordIncrement)) {
        constexpr MachineWord Mask = static_cast<MachineWord>(UINT64_C(0x8080808080808080));
        while (ii < isize) {
          if (*reinterpret_cast<const MachineWord*>(iptr + ii) & Mask)
            break;
          for (int i = 0; i < WordIncrement; ++i)
            optr[oi + i] = static_cast<T>(iptr[ii + i]);
          ii += WordIncrement;
          oi += WordIncrement;
        }
      } else {
        ++ii;
        optr[oi++] = static_cast<char>(head);
      }
    } else {
      ConvertLongCode(iptr, ii, optr, oi, saw_error);
    }
  }

  while (ii < isize) {
    if (iptr[ii] < 0x80) {
      optr[oi++] = iptr[ii++];
    } else {
      if (state.Feed(iptr, ii, isize))
        state.Write(optr, oi, saw_error);
    }
  }
  if (flush && state.NeedsFlush())
    state.Write(optr, oi, saw_error);

  context.maybeThrow(saw_error);
  ASSERT(oi <= output.size());
  return oi;
}

int Encode(Context& context, StringSpan input, MutableBufferSpan output) {
  auto* iptr = reinterpret_cast<const byte_t*>(input.data());
  int isize = input.size();
  auto* optr = reinterpret_cast<char*>(output.data());
  int osize = output.size();

  context.state.bytes[0] = 0;
  return DecodeTmpl(context, BufferSpan(iptr, isize), MutableStringSpan(optr, osize), true);
}

constexpr auto Build() {
  auto builder = BuildTextCodec("UTF-8", Vtable);
  builder.SetIanaCodepage(106);
  builder.SetWindowsCodepage(65001);
  return builder;
}

} // namespace
*/

constexpr const TextEncodingData Utf8EncodingData;

} // namespace detail
} // namespace stp
