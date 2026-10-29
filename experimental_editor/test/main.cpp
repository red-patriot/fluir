#include <QApplication>

#include <gtest/gtest.h>

// Like QTEST_MAIN: one offscreen QApplication outlives every test and dies before Qt's own statics.
int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
