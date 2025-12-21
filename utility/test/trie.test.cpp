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

TEST(TestTrie, CanRetrieveElements) {
  Trie<int> uut{0, {{"abc", 1}, {"hello", 2}, {"abdd", 3}, {"ad", 4}, {"stuff", 5}}};

  EXPECT_EQ(0, uut.at("asdf"));  // Non contained elements are the default
  EXPECT_EQ(1, uut.at("abc"));
  EXPECT_EQ(2, uut.at("hello"));
  EXPECT_EQ(3, uut.at("abdd"));
  EXPECT_EQ(4, uut.at("ad"));
  EXPECT_EQ(5, uut.at("stuff"));
}
