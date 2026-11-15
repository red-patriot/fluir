#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/utility/diagnostic/code.hpp"
#include "compiler/utility/diagnostic/sink.hpp"

namespace fluir::editor {

  /** A diagnostic::Sink that collects messages into a buffer */
  class CollectingSink : public diagnostic::Sink {
   public:
    CollectingSink() = default;

    /** The formatted diagnostics collected so far. */
    const std::vector<std::string>& messages() const { return messages_; }

    /** True when no messages have been collected. */
    bool empty() const { return messages_.empty(); }

   private:
    void report(diagnostic::Code code,
                const std::filesystem::path& file,
                const Sink::ErrorLocation& location,
                std::string_view extraMsg) override;

    std::vector<std::string> messages_;
  };

}  // namespace fluir::editor
