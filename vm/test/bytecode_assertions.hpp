#ifndef FLUIR_VM_TEST_BYTECODE_ASSERTIONS_HPP
#define FLUIR_VM_TEST_BYTECODE_ASSERTIONS_HPP

#include <gtest/gtest.h>

#define EXPECT_BC_HEADER_EQ(expected, actual)    \
  EXPECT_EQ(expected.filetype, actual.filetype); \
  EXPECT_EQ(expected.major, actual.major);       \
  EXPECT_EQ(expected.minor, actual.minor);       \
  EXPECT_EQ(expected.patch, actual.patch);       \
  EXPECT_EQ(expected.entryOffset, actual.entryOffset)

#define EXPECT_BC_VALUES_EQ(expectedValues, actualValues) \
  EXPECT_EQ(expectedValues.size(), actualValues.size());  \
  for (size_t bc_values_eq_i = 0;                         \
       bc_values_eq_i != expectedValues.size();           \
       ++bc_values_eq_i) {                                \
    EXPECT_DOUBLE_EQ(expectedValues[bc_values_eq_i],      \
                     actualValues.at(bc_values_eq_i))     \
        << bc_values_eq_i;                                \
  }

#define EXPECT_CHUNK_CODE_EQ(expectedChunk, actualChunk) \
  EXPECT_EQ(expectedChunk.code, actualChunk.code)

#define EXPECT_CHUNK_EQ(expectedChunk, actualChunk)                    \
  EXPECT_EQ(expectedChunk.name, actualChunk.name);                     \
  EXPECT_BC_VALUES_EQ(expectedChunk.constants, actualChunk.constants); \
  EXPECT_CHUNK_CODE_EQ(expectedChunk, actualChunk)

#endif
