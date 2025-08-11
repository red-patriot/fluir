#include <gtest/gtest.h>

#include <algorithm>
#include <optional>
#include <vector>

#include "compiler/utility/context.hpp"
#include "compiler/utility/pass.hpp"

namespace {
  class MockPass1 {
   public:
    static std::optional<int> run(fluir::Context& ctx, int a) { return a + 1; }
  };

  class MockPass2 {
   public:
    static std::optional<std::vector<double>> run(fluir::Context& ctx, const std::vector<int>& a) {
      std::vector<double> result(a.size(), 0.0);
      std::ranges::transform(a, result.begin(), [](int i) { return static_cast<double>(i); });
      return result;
    }
  };

  class MockPass3 {
   public:
    static std::optional<std::vector<int>> run(fluir::Context& ctx, int a) { return std::vector{a, a + 1, a + 2}; }
  };

  class ReportsErrors {
   public:
    static std::optional<int> run(fluir::Context& ctx, int a) {
      ctx.diagnostics.emitError("Something went wrong.");
      return a - 1;
    }
  };
}  // namespace

TEST(TestCompilerPass, PackageDataAndContextTogether) {
  fluir::Context ctx;
  int a = 1;

  auto results = fluir::addContext(ctx, a);

  EXPECT_EQ(1, results.data);
  EXPECT_TRUE(results.ctx.diagnostics.empty());
}

TEST(TestCompilerPass, PipeDataThroughOnePass) {
  fluir::Context ctx;
  int a = 1;

  auto results = fluir::addContext(ctx, a) | MockPass1::run;

  EXPECT_EQ(2, results.data);
  EXPECT_TRUE(results.ctx.diagnostics.empty());
}

TEST(TestCompilerPass, PipeDataThroughMultiplePasses) {
  fluir::Context ctx;
  int a = 1;
  std::vector<double> expected{2.0, 3.0, 4.0};

  auto results = fluir::addContext(ctx, a) | MockPass1::run | MockPass3::run | MockPass2::run;

  EXPECT_EQ(expected, results.data);
  EXPECT_TRUE(results.ctx.diagnostics.empty());
}

TEST(TestCompilerPass, StopsEarlyOnError) {
  fluir::Context ctx;
  int a = 1;

  auto results = fluir::addContext(ctx, a) | MockPass1::run | ReportsErrors::run | MockPass3::run | MockPass2::run;

  EXPECT_FALSE(results.data.has_value());
  EXPECT_FALSE(results.ctx.diagnostics.empty());
}
