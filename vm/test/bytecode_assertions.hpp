#ifndef FLUIR_VM_TEST_BYTECODE_ASSERTIONS_HPP
#define FLUIR_VM_TEST_BYTECODE_ASSERTIONS_HPP

#include <gtest/gtest.h>

#define EXPECT_BC_HEADER_EQ(expected, actual)    \
  EXPECT_EQ(expected.filetype, actual.filetype); \
  EXPECT_EQ(expected.major, actual.major);       \
  EXPECT_EQ(expected.minor, actual.minor);       \
  EXPECT_EQ(expected.patch, actual.patch);       \
  EXPECT_EQ(expected.entryOffset, actual.entryOffset)

#define EXPECT_BC_VALUES_EQ(expectedValues, actualValues)         \
  EXPECT_EQ(expectedValues.size(), actualValues.size());          \
  for (size_t i = 0; i != expectedValues.size(); ++i) {           \
    EXPECT_DOUBLE_EQ(expectedValues[i], actualValues.at(i)) << i; \
  }

#define EXPECT_CHUNK_CODE_EQ(expectedChunk, actualChunk) \
  EXPECT_EQ(expectedChunk.code, actualChunk.code)

#endif
