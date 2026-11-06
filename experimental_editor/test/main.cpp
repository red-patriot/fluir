#include <QApplication>
#include <QFontDatabase>

#include <gtest/gtest.h>

// Like QTEST_MAIN: one offscreen QApplication outlives every test and dies before Qt's own statics.
// Registers the editor's font as the app does
int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);
  QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
