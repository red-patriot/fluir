#include "editor/app/splash_widget.hpp"

#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

namespace {

  using fluir::editor::SplashWidget;

  QPushButton* button(SplashWidget& splash, const char* name) {
    auto* found = splash.findChild<QPushButton*>(name);
    EXPECT_NE(found, nullptr) << name;
    return found;
  }

}  // namespace

TEST(SplashWidget, NewAsksForANewModule) {
  SplashWidget uut;
  QSignalSpy requested(&uut, &SplashWidget::newRequested);

  QTest::mouseClick(button(uut, "new"), Qt::LeftButton);

  EXPECT_EQ(requested.count(), 1);
}

TEST(SplashWidget, OpenAsksToOpenAModule) {
  SplashWidget uut;
  QSignalSpy requested(&uut, &SplashWidget::openRequested);

  QTest::mouseClick(button(uut, "open"), Qt::LeftButton);

  EXPECT_EQ(requested.count(), 1);
}
