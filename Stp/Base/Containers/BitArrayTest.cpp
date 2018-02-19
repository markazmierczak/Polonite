// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BitArray.h"

#include "Base/Math/Bits.h"
#include "Base/Test/GTest.h"

namespace stp {

static_assert(TIsZeroConstructible<BitArray<257>>, "!");
static_assert(TIsTriviallyRelocatable<BitArray<257>>, "!");

TEST(BitArrayTest, Ctors) {
  {
    constexpr BitArray<256> ba = BitArray<256>();
    EXPECT_FALSE(ba.anyTrue());
  }

  {
    constexpr BitArray<256> ba(UINT64_C(0xAA55AA55AA55AA55));
    EXPECT_EQ(32 * 4, ba.countSetBits());
  }
}

TEST(BitArrayTest, test) {
  {
    constexpr BitArray<7> ba(UINT64_C(0x53));
    EXPECT_TRUE(ba[0]);
    EXPECT_TRUE(ba[1]);
    EXPECT_FALSE(ba[2]);
    EXPECT_FALSE(ba[3]);
    EXPECT_TRUE(ba[4]);
    EXPECT_FALSE(ba[5]);
    EXPECT_TRUE(ba[6]);
  }

  {
    constexpr BitArray<256> ba(UINT64_C(0xAA55AA55AA55AA55));
    for (int i = 0; i < 256 / 4; ++i) {
      int n = i * 4;
      if ((i & 3) < 2) {
        EXPECT_FALSE(ba.testBit(n + 3));
        EXPECT_TRUE(ba.testBit(n + 2));
        EXPECT_FALSE(ba.testBit(n + 1));
        EXPECT_TRUE(ba.testBit(n + 0));
      } else {
        EXPECT_TRUE(ba.testBit(n + 3));
        EXPECT_FALSE(ba.testBit(n + 2));
        EXPECT_TRUE(ba.testBit(n + 1));
        EXPECT_FALSE(ba.testBit(n + 0));
      }
    }
  }
}

TEST(BitArrayTest, SetBit) {
  {
    BitArray<255> ba(UINT64_C(0x55AA55AA55AA55AA));
    for (int i = 0; i < 255; ++i) {
      ba[i] = false;
      EXPECT_FALSE(ba[i]);
    }
  }
  {
    BitArray<255> ba(UINT64_C(0x55AA55AA55AA55AA));
    for (int i = 0; i < 255; ++i) {
      ba[i] = true;
      EXPECT_TRUE(ba[i]);
    }
  }
}

TEST(BitArrayTest, TestArray) {
  BitArray<255> lhs(UINT64_C(0xAA55AA55AA55AA55));
  BitArray<255> rhs(UINT64_C(0x55AA55AA55AA55AA));

  lhs[72] = 1;
  EXPECT_TRUE(lhs.testBit(rhs));
}

TEST(BitArrayTest, Compare) {
  {
    BitArray<255> lhs(UINT64_C(0xAA55AA55AA55AA55));
    BitArray<255> rhs(UINT64_C(0xAA55AA55AA55AA55));
    EXPECT_EQ(rhs, lhs);
    lhs[0] = false;
    EXPECT_NE(rhs, lhs);
  }
}

TEST(BitArrayTest, Count) {
  {
    BitArray<255> ba(UINT64_C(0xAA55AA55AA55AA55));
    EXPECT_EQ(127, ba.countSetBits());
  }
  {
    BitArray<255> ba(0);
    EXPECT_EQ(0, ba.countSetBits());
  }
  {
    BitArray<255> ba;
    ba.setAll();
    EXPECT_EQ(255, ba.countSetBits());
  }
}

TEST(BitArrayTest, ChangeAll) {
  {
    BitArray<255> ba(0xF);
    ba.setAll();
    for (int i = 0; i < 255; ++i)
      EXPECT_TRUE(ba[i]);
  }
  {
    BitArray<255> ba(UINT64_C(0x7F00FF00FF00FF00));
    ba.unsetAll();
    for (int i = 0; i < 255; ++i)
      EXPECT_FALSE(ba[i]);
    EXPECT_EQ(BitArray<255>(), ba);
  }
  {
    BitArray<63> ba(UINT64_C(0x55AA55AA55AA55AA));
    BitArray<63> te(UINT64_C(0x2A55AA55AA55AA55));
    ba.flipAll();
    EXPECT_EQ(te, ba);
  }
}

TEST(BitArrayTest, ChangeOne) {
  {
    BitArray<255> ba(0);
    ba.setBit(130);
    for (int i = 0; i < 255; ++i)
      EXPECT_EQ(i == 130, ba[i]);
  }
  {
    BitArray<255> ba(UINT64_C(0xFFFFFFFFFFFFFFFF));
    ba.unsetBit(130);
    for (int i = 0; i < 255; ++i)
      EXPECT_EQ(i != 130, ba[i]);
  }
  {
    BitArray<255> ba(UINT64_C(0xFFFFFFFFFFFFFFFF));
    ba[129] = 0;
    ba.flipBit(130);
    ba.flipBit(129);
    for (int i = 0; i < 255; ++i)
      EXPECT_EQ(i != 130, ba[i]);
  }
}

TEST(BitArrayTest, FindFirstNextSet) {
  BitArray<255> ba;
  EXPECT_EQ(-1, ba.findFirstSet());
  EXPECT_EQ(-1, ba.findNextSet(0));

  ba[0] = 1;
  EXPECT_EQ(0, ba.findFirstSet());
  EXPECT_EQ(-1, ba.findNextSet(0));

  ba[1] = 1;
  EXPECT_EQ(0, ba.findFirstSet());
  EXPECT_EQ(1, ba.findNextSet(0));
  EXPECT_EQ(-1, ba.findNextSet(1));

  ba[0] = 0;
  EXPECT_EQ(1, ba.findFirstSet());
  EXPECT_EQ(1, ba.findNextSet(0));
  EXPECT_EQ(-1, ba.findNextSet(1));

  ba[254] = 1;
  EXPECT_EQ(1, ba.findFirstSet());
  EXPECT_EQ(254, ba.findNextSet(1));
}

TEST(BitArrayTest, findLastPrevSet) {
  BitArray<255> ba;
  EXPECT_EQ(-1, ba.findLastSet());
  EXPECT_EQ(-1, ba.findPrevSet(254));

  ba[0] = 1;
  EXPECT_EQ(0, ba.findLastSet());
  EXPECT_EQ(-1, ba.findPrevSet(0));

  ba[1] = 1;
  EXPECT_EQ(1, ba.findLastSet());
  EXPECT_EQ(0, ba.findPrevSet(1));
  EXPECT_EQ(-1, ba.findPrevSet(0));

  ba[0] = 0;
  EXPECT_EQ(1, ba.findLastSet());
  EXPECT_EQ(1, ba.findPrevSet(2));
  EXPECT_EQ(-1, ba.findPrevSet(1));

  ba[254] = 1;
  EXPECT_EQ(254, ba.findLastSet());
  EXPECT_EQ(1, ba.findPrevSet(254));
}

TEST(BitArrayTest, Shift) {
  {
    BitArray<255> ba(UINT64_C(0xAA55AA55AA55AA55));
    ba <<= 15;

    BitArray<255> test0(RotateBitsLeft(UINT64_C(0xAA55AA55AA55AA55), 15));
    for (int i = 0; i < 15; ++i)
      test0[i] = 0;
    EXPECT_EQ(test0, ba);

    ba >>= 30;

    BitArray<255> test1(RotateBitsRight(UINT64_C(0xAA55AA55AA55AA55), 15));
    for (int i = 0; i < 30; ++i)
      test1[254 - i] = 0;
    EXPECT_EQ(test1, ba);

    ba <<= 15;

    BitArray<255> test2(UINT64_C(0xAA55AA55AA55AA55));
    for (int i = 0; i < 15; ++i) {
      test2[i] = 0;
      test2[254 - i] = 0;
    }
    EXPECT_EQ(test2, ba);
  }
}

TEST(BitArrayTest, Or) {
  BitArray<256> lhs(UINT64_C(0xAA55AA55AA55AA55));
  BitArray<256> rhs(UINT64_C(0x55AA55AA55AA55AA));
  BitArray<256> result(UINT64_C(0xFFFFFFFFFFFFFFFF));

  lhs |= rhs;
  EXPECT_EQ(result, lhs);
}

TEST(BitArrayTest, Xor) {
  BitArray<256> lhs(UINT64_C(0xAAAAAAAAAAAAAAAA));
  BitArray<256> rhs(UINT64_C(0x55AA55AA55AA55AA));
  BitArray<256> result(UINT64_C(0xFF00FF00FF00FF00));

  lhs ^= rhs;
  EXPECT_EQ(result, lhs);
}

TEST(BitArrayTest, And) {
  BitArray<256> lhs(UINT64_C(0xAAAAAAAAAAAAAAAA));
  BitArray<256> rhs(UINT64_C(0x55AA55AA55AA55AA));
  BitArray<256> result(UINT64_C(0x00AA00AA00AA00AA));

  lhs &= rhs;
  EXPECT_EQ(result, lhs);
}

TEST(BitArrayTest, Neg) {
  {
    BitArray<256> pba(UINT64_C(0xAA55AA55AA55AA55));
    BitArray<256> nba(UINT64_C(0x55AA55AA55AA55AA));
    EXPECT_EQ(nba, ~pba);
  }
  {
    // Test for partial words.
    BitArray<31> pba(0x55AA55AAu);
    BitArray<31> nba(0x2A55AA55u);
    EXPECT_EQ(nba, ~pba);
  }
}

TEST(BitArrayTest, AllAny) {
  {
    BitArray<256> ba(UINT64_C(0xAA55AA55AA55AA55));
    EXPECT_FALSE(ba.allTrue());
    EXPECT_TRUE(ba.anyTrue());
  }
  {
    BitArray<256> ba(0);
    EXPECT_FALSE(ba.allTrue());
    EXPECT_FALSE(ba.anyTrue());
  }
  {
    BitArray<256> ba(0);
    ba[255] = true;
    EXPECT_FALSE(ba.allTrue());
    EXPECT_TRUE(ba.anyTrue());
  }
  {
    BitArray<256> ba(0);
    ba[1] = true;
    EXPECT_FALSE(ba.allTrue());
    EXPECT_TRUE(ba.anyTrue());
  }
  {
    BitArray<256> ba(UINT64_C(0xFFFFFFFFFFFFFFFF));
    EXPECT_TRUE(ba.allTrue());
    EXPECT_TRUE(ba.anyTrue());
  }
}

TEST(BitArrayTest, Swap) {
  BitArray<256> test(UINT64_C(0xAA55AA55AA55AA55));
  BitArray<256> source = test;

  BitArray<256> destination;
  swap(source, destination);

  EXPECT_EQ(destination, test);
  EXPECT_FALSE(source.anyTrue());
}

} // namespace stp
