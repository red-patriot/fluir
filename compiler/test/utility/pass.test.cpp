#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

import fluir.utility.context;
import fluir.utility.pass;

namespace {
  class MockPass1 {
   public:
    static int run(fluir::Context& ctx, int a) { return a + 1; }
  };

  class MockPass2 {
   public:
    static std::vector<double> run(fluir::Context& ctx, const std::vector<int>& a) {
      std::vector<double> result(a.size(), 0.0);
      std::ranges::transform(a, result.begin(), [](int i) { return static_cast<double>(i); });
      return result;
    }
  };

  class MockPass3 {
   public:
    static std::vector<int> run(fluir::Context& ctx, int a) { return {a, a + 1, a + 2}; }
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
