#include <deque>
#include <memory>
#include <doctest/doctest.h>
#include <QLineEdit>
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Tests;

namespace {
  struct CodeModel : TextModel {
    LocalTextModel m_value;

    QValidator::State get_state() const override {
      if(m_value.get().size() == 3) {
        return QValidator::Acceptable;
      }
      return QValidator::Intermediate;
    }

    const QString& get() const override {
      return m_value.get();
    }

    QValidator::State test(const QString& value) const override {
      if(value.size() > 3) {
        return QValidator::Invalid;
      }
      for(auto character : value) {
        if(!character.isLetter()) {
          return QValidator::Invalid;
        }
      }
      return QValidator::Acceptable;
    }

    QValidator::State set(const QString& value) override {
      if(test(value) == QValidator::Invalid) {
        return QValidator::Invalid;
      }
      return m_value.set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_value.connect_update_signal(slot);
    }
  };

  void activate(QWidget& window) {
    window.show();
    window.activateWindow();
    QApplication::processEvents();
    REQUIRE(window.isActiveWindow());
  }

  QWidget& focus(TextBox& text_box) {
    text_box.setFocus(Qt::MouseFocusReason);
    REQUIRE(QApplication::focusWidget());
    return *QApplication::focusWidget();
  }
}

TEST_SUITE("TextBox") {
  TEST_CASE("typing") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(&window);
      activate(window);
      auto& input = focus(*text_box);
      type_text(input, "abc");
      REQUIRE(text_box->get_current()->get() == "abc");
      text_box->get_current()->set("xyz");
      type_text(input, "w");
      REQUIRE(text_box->get_current()->get() == "xyzw");
    });
  }

  TEST_CASE("submit") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(&window);
      auto other = new QWidget(&window);
      other->setFocusPolicy(Qt::StrongFocus);
      auto submissions = std::deque<QString>();
      text_box->connect_submit_signal([&] (const auto& submission) {
        submissions.push_back(submission);
      });
      activate(window);
      auto& input = focus(*text_box);
      press_key(input, Qt::Key_Return);
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front().isEmpty());
      submissions.pop_front();
      type_text(input, "abc");
      press_key(input, Qt::Key_Return);
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front() == "abc");
      submissions.pop_front();
      REQUIRE(text_box->get_submission() == "abc");
      press_key(input, Qt::Key_Return);
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front() == "abc");
      submissions.pop_front();
      type_text(input, "d");
      other->setFocus(Qt::MouseFocusReason);
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front() == "abcd");
      submissions.pop_front();
      REQUIRE(text_box->get_submission() == "abcd");
      focus(*text_box);
      other->setFocus(Qt::MouseFocusReason);
      REQUIRE(submissions.empty());
    });
  }

  TEST_CASE("reject") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(std::make_shared<CodeModel>(), &window);
      auto submissions = std::deque<QString>();
      text_box->connect_submit_signal([&] (const auto& submission) {
        submissions.push_back(submission);
      });
      auto rejections = std::deque<QString>();
      text_box->connect_reject_signal([&] (const auto& value) {
        rejections.push_back(value);
      });
      activate(window);
      auto& input = focus(*text_box);
      type_text(input, "ab");
      press_key(input, Qt::Key_Return);
      REQUIRE(submissions.empty());
      REQUIRE(rejections.size() == 1);
      REQUIRE(rejections.front() == "ab");
      rejections.pop_front();
      REQUIRE(text_box->get_current()->get().isEmpty());
      REQUIRE(is_match(*text_box, Rejected()));
      type_text(input, "a");
      REQUIRE(!is_match(*text_box, Rejected()));
      type_text(input, "bc");
      press_key(input, Qt::Key_Return);
      REQUIRE(rejections.empty());
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front() == "abc");
      submissions.pop_front();
    });
  }

  TEST_CASE("validator") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(std::make_shared<CodeModel>(), &window);
      activate(window);
      auto& input = focus(*text_box);
      type_text(input, "a1b");
      REQUIRE(text_box->get_current()->get() == "ab");
      type_text(input, "cd");
      REQUIRE(text_box->get_current()->get() == "abc");
    });
  }

  TEST_CASE("escape") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(&window);
      activate(window);
      auto& input = focus(*text_box);
      type_text(input, "abc");
      press_key(input, Qt::Key_Return);
      type_text(input, "de");
      REQUIRE(text_box->get_current()->get() == "abcde");
      press_key(input, Qt::Key_Escape);
      REQUIRE(text_box->get_current()->get() == "abc");
      REQUIRE(text_box->get_submission() == "abc");
    });
  }

  TEST_CASE("read_only") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(QString("abc"), &window);
      auto submissions = std::deque<QString>();
      text_box->connect_submit_signal([&] (const auto& submission) {
        submissions.push_back(submission);
      });
      text_box->set_read_only(true);
      REQUIRE(text_box->is_read_only());
      REQUIRE(is_match(*text_box, ReadOnly()));
      activate(window);
      auto& input = focus(*text_box);
      text_box->get_highlight()->set(Highlight(3));
      type_text(input, "d");
      press_key(input, Qt::Key_Return);
      REQUIRE(text_box->get_current()->get() == "abc");
      REQUIRE(submissions.empty());
      text_box->set_read_only(false);
      REQUIRE(!text_box->is_read_only());
      REQUIRE(!is_match(*text_box, ReadOnly()));
      text_box->get_highlight()->set(Highlight(3));
      type_text(input, "d");
      press_key(input, Qt::Key_Return);
      REQUIRE(text_box->get_current()->get() == "abcd");
      REQUIRE(submissions.size() == 1);
      REQUIRE(submissions.front() == "abcd");
      submissions.pop_front();
    });
  }

  TEST_CASE("highlight") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(QString("hello"), &window);
      activate(window);
      auto& input = focus(*text_box);
      auto& highlight = *text_box->get_highlight();
      highlight.set(Highlight(1, 4));
      type_text(input, "X");
      REQUIRE(text_box->get_current()->get() == "hXo");
      REQUIRE(highlight.get() == Highlight(2));
      press_key(input, Qt::Key_End);
      REQUIRE(highlight.get() == Highlight(3));
      press_key(input, Qt::Key_Home);
      REQUIRE(highlight.get() == Highlight(0));
      highlight.set(Highlight(3));
      type_text(input, "Y");
      REQUIRE(text_box->get_current()->get() == "hXoY");
    });
  }

  TEST_CASE("deferred_line_edit") {
    run_ui_test([] {
      auto window = QWidget();
      auto text_box = new TextBox(&window);
      auto label = make_label(QString("Label"), &window);
      auto enabled_label = make_label(QString("Label"), &window);
      auto editable_label = make_label(QString("Label"), &window);
      window.show();
      QApplication::processEvents();
      REQUIRE(text_box->findChild<QLineEdit*>());
      REQUIRE(!label->findChild<QLineEdit*>());
      REQUIRE(!enabled_label->findChild<QLineEdit*>());
      REQUIRE(!editable_label->findChild<QLineEdit*>());
      enabled_label->setEnabled(true);
      REQUIRE(enabled_label->findChild<QLineEdit*>());
      editable_label->set_read_only(false);
      REQUIRE(editable_label->findChild<QLineEdit*>());
      REQUIRE(label->get_submission() == "Label");
      REQUIRE(label->get_current()->get() == "Label");
    });
  }

  TEST_CASE("elided_text") {
    run_ui_test([] {
      auto window = QWidget();
      auto text = QString("The quick brown fox jumps over the lazy dog");
      auto text_box = new TextBox(text, &window);
      text_box->set_read_only(true);
      text_box->setFixedWidth(60);
      activate(window);
      auto line_edit = text_box->findChild<QLineEdit*>();
      REQUIRE(line_edit);
      REQUIRE(line_edit->text() != text);
      REQUIRE(line_edit->text().endsWith(QChar(0x2026)));
      REQUIRE(text_box->get_current()->get() == text);
      text_box->set_read_only(false);
      auto& input = focus(*text_box);
      REQUIRE(line_edit->text() == text);
      text_box->get_highlight()->set(Highlight(text.size()));
      type_text(input, "s");
      REQUIRE(text_box->get_current()->get() == text + "s");
    });
  }
}
