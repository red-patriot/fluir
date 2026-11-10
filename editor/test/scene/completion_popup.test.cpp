#include "editor/scene/completion_popup.hpp"

#include <optional>
#include <QAbstractItemModel>
#include <QApplication>
#include <QLineEdit>
#include <QListView>
#include <QStringList>
#include <QTest>

#include <gtest/gtest.h>

namespace {

  using fluir::editor::CompletionPopup;

  const QStringList kLabels{"+ (binary)", "print", "add (fn)", "Comment", "PRINTER (fn)"};

  struct Popped {
    std::optional<qsizetype> picked;
    int picks = 0;

    Popped() {
      fluir::editor::popUpCompletions(kLabels, QPoint(100, 100), [this](qsizetype index) {
        picked = index;
        ++picks;
      });
    }

    ~Popped() {
      if (QWidget* open = popup()) {
        open->close();
      }
    }

    static CompletionPopup* popup() { return dynamic_cast<CompletionPopup*>(QApplication::activePopupWidget()); }
    static QLineEdit& filter() { return *popup()->findChild<QLineEdit*>(); }
    static QListView& list() { return *popup()->findChild<QListView*>(); }

    static QStringList shown() {
      QStringList out;
      const QAbstractItemModel& model = *list().model();
      for (int row = 0; row < model.rowCount(); ++row) {
        out << model.index(row, 0).data().toString();
      }
      return out;
    }

    static void type(const QString& text) { QTest::keyClicks(&filter(), text); }
    static void key(Qt::Key key) { QTest::keyClick(&filter(), key); }
  };

}  // namespace

TEST(CompletionPopup, ShowsEveryLabelUntilTyping) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);
  EXPECT_EQ(Popped::shown(), kLabels);
}

TEST(CompletionPopup, TypingFiltersBySubstringInAnyCase) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);

  Popped::type("rint");
  EXPECT_EQ(Popped::shown(), (QStringList{"print", "PRINTER (fn)"}));

  Popped::filter().clear();
  Popped::type("FN");
  EXPECT_EQ(Popped::shown(), (QStringList{"add (fn)", "PRINTER (fn)"}));
}

TEST(CompletionPopup, EnterPicksTheTopMatchAndCloses) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);

  Popped::type("fn");
  Popped::key(Qt::Key_Return);

  EXPECT_EQ(p.picked, kLabels.indexOf("add (fn)"));
  EXPECT_EQ(p.picks, 1);
  EXPECT_EQ(Popped::popup(), nullptr);
}

TEST(CompletionPopup, DownAndUpMoveTheHighlight) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);

  Popped::type("fn");
  Popped::key(Qt::Key_Down);
  Popped::key(Qt::Key_Down);
  Popped::key(Qt::Key_Up);
  Popped::key(Qt::Key_Down);
  Popped::key(Qt::Key_Return);

  EXPECT_EQ(p.picked, kLabels.indexOf("PRINTER (fn)"));
}

TEST(CompletionPopup, ClickingARowPicksIt) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);
  QListView& list = Popped::list();
  const QRect row = list.visualRect(list.model()->index(3, 0));

  QTest::mouseClick(list.viewport(), Qt::LeftButton, {}, row.center());

  EXPECT_EQ(p.picked, kLabels.indexOf("Comment"));
  EXPECT_EQ(Popped::popup(), nullptr);
}

TEST(CompletionPopup, EnterWithNoMatchPicksNothingAndStaysOpen) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);

  Popped::type("zzz");
  Popped::key(Qt::Key_Return);

  EXPECT_EQ(p.picks, 0);
  EXPECT_NE(Popped::popup(), nullptr);
}

TEST(CompletionPopup, EscapeClosesWithoutPicking) {
  Popped p;
  ASSERT_NE(Popped::popup(), nullptr);

  Popped::key(Qt::Key_Escape);

  EXPECT_EQ(p.picks, 0);
  EXPECT_EQ(Popped::popup(), nullptr);
}
