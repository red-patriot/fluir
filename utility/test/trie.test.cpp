#include "fluir/util/trie.hpp"

#include <gtest/gtest.h>

using fluir::util::Trie;

TEST(TestTrie, ContainsWithOneElement) {
  Trie<int> uut{0, {{"af", 1}}};

  EXPECT_TRUE(uut.contains("af"));
  EXPECT_FALSE(uut.contains("a"));
}
