#include "editor/core/editor_context.hpp"

#include <gtest/gtest.h>

TEST(EditorContext, DefaultsMatchLegacyConstants) {
  const fluir::editor::EditorContext ctx;

  // Raw layout fields.
  EXPECT_DOUBLE_EQ(ctx.layout.unitPx, 5.0);
  EXPECT_DOUBLE_EQ(ctx.layout.textPad, 4.0);
  EXPECT_DOUBLE_EQ(ctx.layout.portDot, 6.0);
  EXPECT_DOUBLE_EQ(ctx.layout.headerUnits, 5.0);
  EXPECT_DOUBLE_EQ(ctx.layout.railUnits, 5.0);
  EXPECT_DOUBLE_EQ(ctx.layout.paramUnits, 15.0);
  EXPECT_DOUBLE_EQ(ctx.layout.returnInsetUnits, 5.0);

  // Derived layout metrics reproduce the old k* constants.
  EXPECT_DOUBLE_EQ(ctx.layout.headerH(), 25.0);
  EXPECT_DOUBLE_EQ(ctx.layout.railStep(), 25.0);
  EXPECT_DOUBLE_EQ(ctx.layout.paramW(), 75.0);

  // Theme colors mirror the SdlRenderer literals.
  EXPECT_EQ(ctx.theme.background, (fluir::editor::Color{24, 26, 31, 255}));
  EXPECT_EQ(ctx.theme.rectStroke, (fluir::editor::Color{200, 200, 210, 255}));
  EXPECT_EQ(ctx.theme.fill, (fluir::editor::Color{120, 120, 140, 255}));
  EXPECT_EQ(ctx.theme.line, (fluir::editor::Color{150, 180, 220, 255}));
  EXPECT_EQ(ctx.theme.text, (fluir::editor::Color{225, 225, 235, 255}));

  // Zoom knobs.
  EXPECT_DOUBLE_EQ(ctx.zoom.min, 0.25);
  EXPECT_DOUBLE_EQ(ctx.zoom.max, 2.5);
  EXPECT_DOUBLE_EQ(ctx.zoom.wheelStep, 1.1);

  // Window size.
  EXPECT_EQ(ctx.window.width, 1280);
  EXPECT_EQ(ctx.window.height, 800);
}
