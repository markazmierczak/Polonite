// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.



namespace stp {

template<typename T>
List<TStringPiece<T>> TStringPiece<T>::SplitToPieces(
    T separator, StringSplitOption option) const {
  List<TStringPiece<T>> result;

  TStringPiece input = *this;
  while (!input.IsEmpty()) {
    int pos = input.IndexOf(separator);
    if (pos < 0)
      pos = input.length();

    TStringPiece part(input.data(), pos);
    if (!part.IsEmpty() || option == StringSplitOption::KeepEmptyParts)
      result.Add(part);

    if (pos == input.length())
      break;

    input.RemovePrefix(pos + 1);
  }
  return result;
}

template<typename T>
List<TStringPiece<T>> TStringPiece<T>::SplitToPieces(
    TStringPiece separators, StringSplitOption option) const {
  List<TStringPiece<T>> result;

  TStringPiece input = *this;
  while (!input.IsEmpty()) {
    int pos = input.IndexOfAny(separators);
    if (pos < 0)
      pos = input.length();

    TStringPiece part(input.data(), pos);
    if (!part.IsEmpty() || option == StringSplitOption::KeepEmptyParts)
      result.Add(part);

    if (pos == input.length())
      break;

    input.RemovePrefix(pos + 1);
  }
  return result;
}

template<typename T>
List<TString<T>> TStringPiece<T>::SplitToStrings(
    T separator, StringSplitOption option) const {
  List<TString<T>> result;

  TStringPiece input = *this;
  while (!input.IsEmpty()) {
    int pos = input.IndexOf(separator);
    if (pos < 0)
      pos = input.length();

    TStringPiece part(input.data(), pos);
    if (!part.IsEmpty() || option == StringSplitOption::KeepEmptyParts)
      result.Add(StringType(part));

    if (pos == input.length())
      break;

    input.RemovePrefix(pos + 1);
  }
  return result;
}

template<typename T>
List<TString<T>> TStringPiece<T>::SplitToStrings(
    TStringPiece separators, StringSplitOption option) const {
  List<TString<T>> result;

  TStringPiece input = *this;
  while (!input.IsEmpty()) {
    int pos = input.IndexOfAny(separators);
    if (pos < 0)
      pos = input.length();

    TStringPiece part(input.data(), pos);
    if (!part.IsEmpty() || option == StringSplitOption::KeepEmptyParts)
      result.Add(StringType(part));

    if (pos == input.length())
      break;

    input.RemovePrefix(pos + 1);
  }
  return result;
}

} // namespace stp
