#include "compiler/backend/inspect_writer.hpp"

#include <gtest/gtest.h>

#include "compiler/backend/bytecode_generator.hpp"

namespace fc = fluir::code;

TEST(TestInspectWriter, WriteEmptyProgram) {
  std::string expected = R"(I010C1100000000000000FF
CHUNK main
  CONSTANTS x0
  CODE x1
    IEXIT
)";

  fc::ByteCode code{
      .header = {.filetype = '\0',
                 .major = 1,
                 .minor = 12,
                 .patch = 17,
                 .entryOffset = 255},
      .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}
