#include <gtest/gtest.h>

#include "compiler/models/ast/node.hpp"
#include "compiler/utility/context.hpp"
#include "test_diagnostic_sink.hpp"

namespace {
  fluir::ast::Dependency i32Constant(std::optional<unsigned> index = std::nullopt) {
    return fluir::ast::createDependency(
      index,
      fluir::ast::createNode<fluir::ast::Constant>(
        fluir::literals_types::I32{1}, fluir::FullID{1, 1}, fluir::FlowGraphLocation{}));
  }
}  // namespace

TEST(TestDependency, UnindexedReportsChildTypeWithoutSetType) {
  const auto uut = i32Constant();

  EXPECT_EQ(fluir::types::ID_I32, uut.type());
}

TEST(TestDependency, UnindexedFollowsChildTypeChanges) {
  auto uut = i32Constant();

  uut.get().setType(fluir::types::ID_I64);

  EXPECT_EQ(fluir::types::ID_I64, uut.type());
}

TEST(TestDependency, SetTypeInvalidDoesNotCrash) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  auto uut = i32Constant();

  uut.setType(fluir::types::ID_INVALID, ctx);

  EXPECT_EQ(fluir::types::ID_INVALID, uut.type());
}

TEST(TestDependency, IndexedResolvesProductElement) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  const auto product =
    ctx.symbolTable.addType(fluir::types::Product("P", {fluir::types::ID_I32, fluir::types::ID_BOOL}));
  auto uut = i32Constant(1);

  uut.setType(product, ctx);

  EXPECT_EQ(fluir::types::ID_BOOL, uut.type());
  EXPECT_EQ(product, uut.fullType());
}
