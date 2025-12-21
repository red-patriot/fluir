#include "fluir/util/trie.hpp"

#include <gtest/gtest.h>

using fluir::util::Trie;

TEST(TestTrie, ContainsWithOneElement) {
  Trie<int> uut{0, {{"af", 1}}};

  EXPECT_TRUE(uut.contains("af"));
  EXPECT_FALSE(uut.contains("a"));
}

TEST(TestTrie, ContainsWithMultipleElementsOfSameLength) {
  Trie<int> uut{0, {{"af", 1}, {"bf", 1}, {"cf", 1}, {"df", 1}}};

  EXPECT_TRUE(uut.contains("af"));
  EXPECT_TRUE(uut.contains("bf"));
  EXPECT_TRUE(uut.contains("cf"));
  EXPECT_TRUE(uut.contains("df"));
  EXPECT_FALSE(uut.contains("ab"));
  EXPECT_FALSE(uut.contains("a"));
}

TEST(TestTrie, ContainsWithMultipleElementsOfDifferingLengths) {
  Trie<int> uut{0, {{"abc", 1}, {"hello", 1}, {"abdd", 1}, {"ad", 1}, {"stuff", 1}}};

  EXPECT_TRUE(uut.contains("abc"));
  EXPECT_TRUE(uut.contains("hello"));
  EXPECT_TRUE(uut.contains("abdd"));
  EXPECT_TRUE(uut.contains("ad"));
  EXPECT_TRUE(uut.contains("stuff"));
  EXPECT_FALSE(uut.contains("ab"));
  EXPECT_FALSE(uut.contains("hell"));
  EXPECT_FALSE(uut.contains("a"));
  EXPECT_FALSE(uut.contains("asdf"));
}
