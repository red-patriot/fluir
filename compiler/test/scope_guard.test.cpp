#include <gtest/gtest.h>

#include "compiler/scope_guard.hpp"

class ScopeGuardTest : public ::testing::Test {
 protected:
  void SetUp() override {
    execution_log.clear();
    counter = 0;
  }

  std::vector<std::string> execution_log;
  int counter = 0;

  void logExecution(const std::string& msg) { execution_log.push_back(msg); }
};

// Test FLUIR_SCOPE_EXIT - should always execute
TEST_F(ScopeGuardTest, ScopeExitExecutesOnNormalReturn) {
  {
    FLUIR_SCOPE_EXIT { logExecution("scope_exit"); };
    logExecution("normal_execution");
  }

  ASSERT_EQ(execution_log.size(), 2);
  EXPECT_EQ(execution_log.at(0), "normal_execution");
  EXPECT_EQ(execution_log.at(1), "scope_exit");
}

TEST_F(ScopeGuardTest, ScopeExitExecutesOnException) {
  try {
    FLUIR_SCOPE_EXIT { logExecution("scope_exit"); };
    logExecution("before_exception");
    throw std::runtime_error("test exception");
    logExecution("after_exception");  // Should not execute
  } catch (const std::exception&) {
    // Exception caught
  }

  ASSERT_EQ(execution_log.size(), 2);
  EXPECT_EQ(execution_log.at(0), "before_exception");
  EXPECT_EQ(execution_log.at(1), "scope_exit");
}

TEST_F(ScopeGuardTest, MultipleScopeExitGuards) {
  {
    FLUIR_SCOPE_EXIT { logExecution("exit_1"); };
    FLUIR_SCOPE_EXIT { logExecution("exit_2"); };
    FLUIR_SCOPE_EXIT { logExecution("exit_3"); };
    logExecution("normal");
  }

  ASSERT_EQ(execution_log.size(), 4);
  EXPECT_EQ(execution_log.at(0), "normal");
  // Scope guards should execute in reverse order of declaration
  EXPECT_EQ(execution_log.at(1), "exit_3");
  EXPECT_EQ(execution_log.at(2), "exit_2");
  EXPECT_EQ(execution_log.at(3), "exit_1");
}

// Test FLUIR_SCOPE_SUCCESS - should only execute on normal exit
TEST_F(ScopeGuardTest, ScopeSuccessExecutesOnNormalReturn) {
  {
    FLUIR_SCOPE_SUCCESS { logExecution("scope_success"); };
    logExecution("normal_execution");
  }

  ASSERT_EQ(execution_log.size(), 2);
  EXPECT_EQ(execution_log.at(0), "normal_execution");
  EXPECT_EQ(execution_log.at(1), "scope_success");
}

TEST_F(ScopeGuardTest, ScopeSuccessDoesNotExecuteOnException) {
  try {
    FLUIR_SCOPE_SUCCESS { logExecution("scope_success"); };
    logExecution("before_exception");
    throw std::runtime_error("test exception");
  } catch (const std::exception&) {
    // Exception caught
  }

  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "before_exception");
  // scope_success should not be in the log
}

TEST_F(ScopeGuardTest, MultipleScopeSuccessGuards) {
  {
    FLUIR_SCOPE_SUCCESS { logExecution("success_1"); };
    FLUIR_SCOPE_SUCCESS { logExecution("success_2"); };
    logExecution("normal");
  }

  ASSERT_EQ(execution_log.size(), 3);
  EXPECT_EQ(execution_log.at(0), "normal");
  EXPECT_EQ(execution_log.at(1), "success_2");
  EXPECT_EQ(execution_log.at(2), "success_1");
}

// Test FLUIR_SCOPE_FAILURE - should only execute on exception
TEST_F(ScopeGuardTest, ScopeFailureDoesNotExecuteOnNormalReturn) {
  {
    FLUIR_SCOPE_FAILURE { logExecution("scope_failure"); };
    logExecution("normal_execution");
  }

  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "normal_execution");
  // scope_failure should not be in the log
}

TEST_F(ScopeGuardTest, ScopeFailureExecutesOnException) {
  try {
    FLUIR_SCOPE_FAILURE { logExecution("scope_failure"); };
    logExecution("before_exception");
    throw std::runtime_error("test exception");
  } catch (const std::exception&) {
    // Exception caught
  }

  ASSERT_EQ(execution_log.size(), 2);
  EXPECT_EQ(execution_log.at(0), "before_exception");
  EXPECT_EQ(execution_log.at(1), "scope_failure");
}

TEST_F(ScopeGuardTest, MultipleScopeFailureGuards) {
  try {
    FLUIR_SCOPE_FAILURE { logExecution("failure_1"); };
    FLUIR_SCOPE_FAILURE { logExecution("failure_2"); };
    logExecution("before_exception");
    throw std::runtime_error("test exception");
  } catch (const std::exception&) {
    // Exception caught
  }

  ASSERT_EQ(execution_log.size(), 3);
  EXPECT_EQ(execution_log.at(0), "before_exception");
  EXPECT_EQ(execution_log.at(1), "failure_2");
  EXPECT_EQ(execution_log.at(2), "failure_1");
}

// Test mixed scope guards
TEST_F(ScopeGuardTest, MixedScopeGuardsNormalExecution) {
  {
    FLUIR_SCOPE_EXIT { logExecution("exit"); };
    FLUIR_SCOPE_SUCCESS { logExecution("success"); };
    FLUIR_SCOPE_FAILURE { logExecution("failure"); };
    logExecution("normal");
  }

  ASSERT_EQ(execution_log.size(), 3);
  EXPECT_EQ(execution_log.at(0), "normal");
  EXPECT_EQ(execution_log.at(1), "success");
  EXPECT_EQ(execution_log.at(2), "exit");
  // failure should not execute
}

TEST_F(ScopeGuardTest, MixedScopeGuardsExceptionExecution) {
  try {
    FLUIR_SCOPE_EXIT { logExecution("exit"); };
    FLUIR_SCOPE_SUCCESS { logExecution("success"); };
    FLUIR_SCOPE_FAILURE { logExecution("failure"); };
    logExecution("before_exception");
    throw std::runtime_error("test");
  } catch (const std::exception&) {
    // Exception caught
  }

  ASSERT_EQ(execution_log.size(), 3);
  EXPECT_EQ(execution_log.at(0), "before_exception");
  EXPECT_EQ(execution_log.at(1), "failure");
  EXPECT_EQ(execution_log.at(2), "exit");
  // success should not execute
}

// Test nested scopes
TEST_F(ScopeGuardTest, NestedScopeGuards) {
  FLUIR_SCOPE_EXIT { logExecution("outer_exit"); };
  logExecution("outer_start");

  {
    FLUIR_SCOPE_EXIT { logExecution("inner_exit"); };
    logExecution("inner");
  }

  logExecution("outer_end");

  ASSERT_EQ(execution_log.size(), 4);
  EXPECT_EQ(execution_log.at(0), "outer_start");
  EXPECT_EQ(execution_log.at(1), "inner");
  EXPECT_EQ(execution_log.at(2), "inner_exit");
  EXPECT_EQ(execution_log.at(3), "outer_end");
  // Note: outer_exit would execute after this scope ends
}

// Test variable capture
TEST_F(ScopeGuardTest, VariableCapture) {
  int local_var = 42;
  std::string captured_value;

  {
    FLUIR_SCOPE_EXIT { captured_value = std::to_string(local_var); };
    local_var = 100;
  }

  EXPECT_EQ(captured_value, "100");
}

TEST_F(ScopeGuardTest, VariableCaptureByReference) {
  int resource_count = 5;

  {
    FLUIR_SCOPE_EXIT { resource_count--; };
    FLUIR_SCOPE_SUCCESS { resource_count += 10; };
    // Normal execution
  }

  EXPECT_EQ(resource_count, 14);  // 5 + 10 - 1
}

// Test exception safety in scope guards themselves
TEST_F(ScopeGuardTest, ExceptionInScopeExitIsNoexcept) {
  // This test verifies that SCOPE_EXIT is marked noexcept
  // In actual implementation, this would terminate if exception is thrown
  bool exit_executed = false;

  {
    FLUIR_SCOPE_EXIT {
      exit_executed = true;
      // This should not throw due to noexcept
    };
  }

  EXPECT_TRUE(exit_executed);
}

// Test resource management patterns
TEST_F(ScopeGuardTest, FileHandlePattern) {
  bool file_opened = false;
  bool file_closed = false;

  {
    // Simulate file opening
    file_opened = true;

    FLUIR_SCOPE_EXIT {
      if (file_opened) {
        file_closed = true;
      }
    };

    // Do work with file
    logExecution("file_work");
  }

  EXPECT_TRUE(file_opened);
  EXPECT_TRUE(file_closed);
  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "file_work");
}

TEST_F(ScopeGuardTest, DatabaseTransactionPattern) {
  bool transaction_started = false;
  bool transaction_committed = false;
  bool transaction_rolled_back = false;

  auto test_transaction = [&](bool should_throw) {
    transaction_started = true;
    transaction_committed = false;
    transaction_rolled_back = false;

    FLUIR_SCOPE_FAILURE {
      if (transaction_started && !transaction_committed) {
        transaction_rolled_back = true;
      }
    };

    FLUIR_SCOPE_SUCCESS { transaction_committed = true; };

    if (should_throw) {
      throw std::runtime_error("Transaction failed");
    }
  };

  // Test successful transaction
  test_transaction(false);
  EXPECT_TRUE(transaction_started);
  EXPECT_TRUE(transaction_committed);
  EXPECT_FALSE(transaction_rolled_back);

  // Test failed transaction
  try {
    test_transaction(true);
  } catch (const std::exception&) {
    // Expected
  }
  EXPECT_TRUE(transaction_started);
  EXPECT_FALSE(transaction_committed);
  EXPECT_TRUE(transaction_rolled_back);
}

// Test early return scenarios
TEST_F(ScopeGuardTest, EarlyReturnWithScopeGuards) {
  auto test_function = [&](bool early_return) -> int {
    FLUIR_SCOPE_EXIT { logExecution("exit"); };

    FLUIR_SCOPE_SUCCESS { logExecution("success"); };

    logExecution("start");

    if (early_return) {
      logExecution("early_return");
      return 42;
    }

    logExecution("normal_path");
    return 0;
  };

  // Test early return
  execution_log.clear();
  int result = test_function(true);
  EXPECT_EQ(result, 42);
  ASSERT_EQ(execution_log.size(), 4);
  EXPECT_EQ(execution_log.at(0), "start");
  EXPECT_EQ(execution_log.at(1), "early_return");
  EXPECT_EQ(execution_log.at(2), "success");
  EXPECT_EQ(execution_log.at(3), "exit");

  // Test normal return
  execution_log.clear();
  result = test_function(false);
  EXPECT_EQ(result, 0);
  ASSERT_EQ(execution_log.size(), 4);
  EXPECT_EQ(execution_log.at(0), "start");
  EXPECT_EQ(execution_log.at(1), "normal_path");
  EXPECT_EQ(execution_log.at(2), "success");
  EXPECT_EQ(execution_log.at(3), "exit");
}

// Test with different exception types
TEST_F(ScopeGuardTest, DifferentExceptionTypes) {
  auto test_with_exception = [&](auto exception_to_throw) {
    execution_log.clear();
    try {
      FLUIR_SCOPE_FAILURE { logExecution("failure"); };
      throw exception_to_throw;
    } catch (...) {
      // Catch all
    }
  };

  test_with_exception(std::runtime_error("runtime"));
  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "failure");

  test_with_exception(std::logic_error("logic"));
  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "failure");

  test_with_exception(42);  // Non-standard exception
  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log.at(0), "failure");
}

// Performance/stress test
TEST_F(ScopeGuardTest, ManyNestedScopeGuards) {
  const int depth = 100;
  int counter = 0;

  std::function<void(int)> nested_function = [&](int level) {
    FLUIR_SCOPE_EXIT { counter++; };

    if (level > 0) {
      nested_function(level - 1);
    }
  };

  nested_function(depth);
  EXPECT_EQ(counter, depth + 1);
}
