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

  class NotAPass1 {
   public:
    static std::vector<int> doRun(fluir::Context& ctx, int a) { return {a, a + 1, a + 2}; }
  };
  class NotAPass2 {
   public:
    static std::vector<int> run(int a) { return {a, a + 1, a + 2}; }
  };
  class NotAPass3 {
   public:
    static std::vector<int> run(fluir::Context& ctx) { return {}; }
  };
}  // namespace

TEST(TestCompilerPass, DetectPassType) {
  EXPECT_TRUE((fluir::CompilerPass<MockPass1, int>));
  EXPECT_TRUE((fluir::CompilerPass<MockPass2, std::vector<int>>));
  EXPECT_TRUE((fluir::CompilerPass<MockPass3, int>));

  EXPECT_FALSE((fluir::CompilerPass<NotAPass1, int>));
  EXPECT_FALSE((fluir::CompilerPass<NotAPass2, int>));
  EXPECT_FALSE((fluir::CompilerPass<NotAPass3, void>));
}
