#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

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
      Color border{200, 200, 210, 255};
      Color conduit{150, 180, 220, 255};
      Color text{225, 225, 235, 255};
      Color funcDeclHeader{237, 170, 30, 255};
      Color operatorNode{75, 107, 210, 255};
      Color uIntNode{242, 44, 189, 255};
      Color sIntNode{225, 31, 251, 255};
      Color floatNode{139, 31, 255, 255};
      Color callNode{31, 117, 255, 255};
    };

    struct Window {
      int width = 1280;
      int height = 800;
    };

    Layout layout;
    Zoom zoom;
    Theme theme;
    Window window;
    std::optional<std::filesystem::path> program;
  };

}  // namespace fluir::editor
