// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_SPLIT_H_
#define STP_BASE_TEXT_SPLIT_H_

#include "Base/Containers/ArrayOps.h"
#include "Base/Containers/InitializerList.h"

namespace stp {

enum class StringSplitOption {
  KeepEmptyParts,
  // Only nonempty results will be added to the results.
  // Multiple separators will be coalesced.
  SkipEmptyParts,
};

// TODO generator

StringPieceListType SplitToPieces(
    T separator,
    StringSplitOption option = StringSplitOption::KeepEmptyParts) const;
StringPieceListType SplitToPieces(
    TStringPiece separators,
    StringSplitOption option = StringSplitOption::KeepEmptyParts) const;

StringListType SplitToStrings(
    T separator,
    StringSplitOption option = StringSplitOption::KeepEmptyParts) const;
StringListType SplitToStrings(
    TStringPiece separators,
    StringSplitOption option = StringSplitOption::KeepEmptyParts) const;

} // namespace stp

#endif // STP_BASE_TEXT_SPLIT_H_
