// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_JOIN_H_
#define STP_BASE_TEXT_JOIN_H_



namespace stp {

template<typename... Ts>
static TString ConcatArgs(const Ts&... parts);

template<typename TArray>
static TString ConcatArray(const TArray& array);

template<typename... Ts>
static TString Join(const Ts&... parts, T separator);

template<typename TArray>
static TString JoinArray(const TArray& array, T separator);

template<typename... Ts>
static TString Join(const Ts&... parts, PieceType separator);

template<typename TArray>
static TString JoinArray(const TArray& array, PieceType separator);

template<typename T>
template<typename... Ts>
inline TString<T> TString<T>::ConcatArgs(const Ts& ... parts) {
  InitializerList<PieceType> ilist = { parts... };
  return ConcatArray(ilist);
}

template<typename T>
template<typename TArray>
TString<T> TString<T>::ConcatArray(const TArray& array) {
  int result_length = 0;
  for (const auto& item : array)
    result_length += item.length();

  TString<T> result;
  T* dst = result.AppendUninitialized(result_length);

  for (const auto& item : array) {
    ArrayCopy(dst, item.data(), item.length());
    dst += item.length();
  }
  return result;
}

template<typename T>
template<typename... Ts>
inline TString<T> TString<T>::Join(const Ts& ... parts, T separator) {
  InitializerList<PieceType> ilist = { parts... };
  return JoinArray(ilist, separator);
}

template<typename T>
template<typename TArray>
TString<T> TString<T>::JoinArray(const TArray& array, T separator) {
  int result_length = array.size() - 1;
  for (const auto& item : array)
    result_length += item.length();

  TString<T> result;
  T* dst = result.AppendUninitialized(result_length);

  for (const auto& item : array) {
    ArrayCopy(dst, item.data(), item.length());
    dst += item.length();
    *dst++ = separator;
  }
  *--dst = '\0';
  return result;
}

template<typename T>
template<typename... Ts>
inline TString<T> TString<T>::Join(const Ts& ... parts, PieceType separator) {
  InitializerList<PieceType> ilist = { parts... };
  return JoinArray(ilist, separator);
}

template<typename T>
template<typename TArray>
TString<T> TString<T>::JoinArray(const TArray& array, PieceType separator) {
  if (UNLIKELY(separator.IsEmpty()))
    return ConcatArgs(array);

  int result_length = 0;
  for (const auto& item : array)
    result_length += item.length();

  result_length += array.size() * separator.length();

  TString<T> result;
  T* dst = result.AppendUninitialized(result_length);

  for (const auto& item : array) {
    ArrayCopy(dst, item.data(), item.length());
    dst += item.length();
    ArrayCopy(dst, separator.data(), separator.length());
    dst += separator.length();
  }
  // Strip last separator.
  if (result_length != 0)
    result.Truncate(result_length - separator.length());
  return result;
}

} // namespace stp

#endif // STP_BASE_TEXT_JOIN_H_
