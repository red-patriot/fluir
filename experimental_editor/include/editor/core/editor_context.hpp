#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>

#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** RGBA color. */
  struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
    friend constexpr bool operator==(const Color&, const Color&) = default;
  };

  /** Tunable knobs for the editor. */
  struct EditorContext {
    struct Layout {
      double unitPx = 5.0;
      double textPad = 4.0;
      double portDot = 6.0;
      double headerUnits = 5.0;
      double railUnits = 5.0;
      double paramUnits = 15.0;
      double returnInsetUnits = 5.0;
      // Gap between a selected actor's rect and its outline, in world px.
      double selectionPad = 2.0;
      // App-level chrome bar height, in screen px (not world units).
      double chromeHeaderPx = 32.0;

      constexpr double headerH() const { return headerUnits * unitPx; }
      constexpr double railStep() const { return railUnits * unitPx; }
      constexpr double paramW() const { return paramUnits * unitPx; }
    };

    struct Zoom {
      double min = 0.25;
      double max = 2.5;
      double wheelStep = 1.1;
    };

    struct Theme {
      Color background{24, 26, 31, 255};
      Color buttonEnabled{75, 107, 210, 255};
      Color buttonDisabled{95, 95, 95, 255};
      Color border{200, 200, 210, 255};
      Color conduit{150, 180, 220, 255};
      Color error{224, 68, 68, 255};
      Color text{225, 225, 235, 255};
      Color funcDeclHeader{237, 170, 30, 255};
      Color operatorNode{75, 107, 210, 255};
      Color uIntNode{242, 44, 189, 255};
      Color sIntNode{225, 31, 251, 255};
      Color floatNode{139, 31, 255, 255};
      Color callNode{31, 117, 255, 255};
      Color headerBackground{34, 37, 45, 255};
    };

    struct Window {
      int width = 1280;
      int height = 800;
    };

    /** Raises a scene edit. Unset outside a live ModulePage, where edits are dropped. */
    using TransactionSink = std::function<bool(std::unique_ptr<Transaction>)>;

    Layout layout;
    Zoom zoom;
    Theme theme;
    Window window;
    bool running = true;
    std::optional<std::filesystem::path> program;
    TransactionSink commit;

    bool dispatch(std::unique_ptr<Transaction> edit) const { return commit && commit(std::move(edit)); }
  };

}  // namespace fluir::editor
