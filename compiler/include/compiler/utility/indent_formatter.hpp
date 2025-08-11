#ifndef FLUIR_COMPILER_UTILITY_INDENT_FORMATTER_HPP
#define FLUIR_COMPILER_UTILITY_INDENT_FORMATTER_HPP

#include <string>

#include "fmt/format.h"

namespace fluir {
  template <unsigned int Level = 2>
  class IndentFormatter {
    class IndentLevel {
      friend IndentFormatter;
      explicit IndentLevel(IndentFormatter* formatter) :
          formatter_(formatter) { }
      IndentLevel(const IndentLevel&) = delete;
      IndentLevel& operator=(const IndentLevel&) = delete;
      IndentLevel(IndentLevel&&) = delete;
      IndentLevel& operator=(IndentLevel&&) = delete;

      IndentFormatter* formatter_;

     public:
      ~IndentLevel() { formatter_->level_ -= 2; }
    };

   public:
    template <typename... FmtArgs>
    std::string
    formatIndented(fmt::format_string<FmtArgs...> format, FmtArgs&&... args) {
      return fmt::format("{}", indentation()) + fmt::format(format, std::forward<FmtArgs>(args)...);
    }

    std::string_view indentation() const { return std::string_view{indent_.c_str(), level_}; }

    [[nodiscard]] IndentLevel indent() {
      level_ += 2;
      if (indent_.size() < level_) {
        indent_ = std::string(level_, ' ');
      }
      return IndentLevel{this};
    }

   private:
    size_t level_{0};
    std::string indent_ = std::string(8, ' ');
  };
}  // namespace fluir

#endif
