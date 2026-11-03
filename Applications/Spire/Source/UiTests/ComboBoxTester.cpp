#include <deque>
#include <memory>
#include <vector>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/EmptyState.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Tests;

namespace {
  auto make_query_model() {
    auto model = std::make_shared<LocalQueryModel<QString>>();
    model->add("Almond");
    model->add("Amber");
    model->add("Apple");
    model->add("Beige");
    model->add("Brown");
    return model;
  }

  void activate(QWidget& window) {
    window.show();
    window.activateWindow();
    QApplication::processEvents();
    REQUIRE(window.isActiveWindow());
  }

  void focus(QWidget& box) {
    box.setFocus(Qt::MouseFocusReason);
    REQUIRE(QApplication::focusWidget());
  }

  QWidget& get_focus() {
    auto widget = QApplication::focusWidget();
    REQUIRE(widget);
    return *widget;
  }

  void flush_queries() {
    QApplication::processEvents(QEventLoop::AllEvents, 100);
  }

  template<typename T>
  T* find_child(QWidget& box) {
    for(auto child : box.findChildren<QWidget*>()) {
      if(auto widget = dynamic_cast<T*>(child)) {
        return widget;
      }
    }
    return nullptr;
  }

  template<typename T>
  bool is_visible(QWidget& box) {
    auto pop_up = find_child<T>(box);
    return pop_up && pop_up->isVisible();
  }

  TextBox& get_text_box(QWidget& box) {
    auto text_box = find_child<TextBox>(box);
    REQUIRE(text_box);
    return *text_box;
  }

  QString get_text(QWidget& box) {
    return get_text_box(box).get_current()->get();
  }

  ListView& get_list_view(QWidget& box) {
    auto drop_down_list = find_child<DropDownList>(box);
    REQUIRE(drop_down_list);
    return drop_down_list->get_list_view();
  }

  std::vector<QString> get_matches(QWidget& box) {
    auto matches = std::vector<QString>();
    auto& list = *get_list_view(box).get_list();
    for(auto i = 0; i != list.get_size(); ++i) {
      matches.push_back(std::any_cast<QString>(list.get(i)));
    }
    return matches;
  }
}

TEST_SUITE("ComboBox") {
  UI_TEST_CASE("query") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    type_text(get_focus(), "a");
    flush_queries();
    REQUIRE(is_visible<DropDownList>(*box));
    REQUIRE(!is_visible<EmptyState>(*box));
    REQUIRE(is_match(*box, PopUp()));
    REQUIRE(get_matches(*box) ==
      std::vector<QString>{"Amber", "Almond", "Apple"});
    REQUIRE(get_text(*box) == "amber");
    REQUIRE(get_text_box(*box).get_highlight()->get() == Highlight(5, 1));
    type_text(get_focus(), "l");
    flush_queries();
    REQUIRE(get_matches(*box) == std::vector<QString>{"Almond"});
    REQUIRE(get_text(*box) == "almond");
    press_key(get_focus(), Qt::Key_Backspace);
    flush_queries();
    REQUIRE(get_text(*box) == "al");
    REQUIRE(is_visible<DropDownList>(*box));
    type_text(get_focus(), "z");
    flush_queries();
    REQUIRE(get_text(*box) == "alz");
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(is_visible<EmptyState>(*box));
    REQUIRE(is_match(*box, PopUp()));
    press_key(get_focus(), Qt::Key_Backspace);
    press_key(get_focus(), Qt::Key_Backspace);
    press_key(get_focus(), Qt::Key_Backspace);
    flush_queries();
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(!is_visible<EmptyState>(*box));
    REQUIRE(!is_match(*box, PopUp()));
  }

  UI_TEST_CASE("navigation") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    press_key(get_focus(), Qt::Key_Down);
    flush_queries();
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(!is_match(*box, PopUp()));
    type_text(get_focus(), "a");
    flush_queries();
    press_key(get_focus(), Qt::Key_Down);
    REQUIRE(get_text(*box) == "Amber");
    REQUIRE(box->get_current()->get() == "Amber");
    press_key(get_focus(), Qt::Key_Down);
    REQUIRE(get_text(*box) == "Almond");
    REQUIRE(box->get_current()->get() == "Almond");
    press_key(get_focus(), Qt::Key_Up);
    REQUIRE(get_text(*box) == "Amber");
    REQUIRE(box->get_current()->get() == "Amber");
    REQUIRE(is_visible<DropDownList>(*box));
    press_key(get_focus(), Qt::Key_Escape);
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(!is_match(*box, PopUp()));
    press_key(get_focus(), Qt::Key_Down);
    REQUIRE(is_visible<DropDownList>(*box));
    REQUIRE(is_match(*box, PopUp()));
  }

  UI_TEST_CASE("submit") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    auto submissions = std::deque<QString>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(submission);
    });
    activate(window);
    focus(*box);
    type_text(get_focus(), "x");
    flush_queries();
    press_key(get_focus(), Qt::Key_Return);
    flush_queries();
    REQUIRE(submissions.empty());
    press_key(get_focus(), Qt::Key_Backspace);
    type_text(get_focus(), "ap");
    flush_queries();
    REQUIRE(get_text(*box) == "apple");
    press_key(get_focus(), Qt::Key_Return);
    flush_queries();
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == "Apple");
    submissions.pop_front();
    REQUIRE(box->get_submission() == "Apple");
    REQUIRE(box->get_current()->get() == "Apple");
    REQUIRE(get_text(*box) == "Apple");
    REQUIRE(!is_visible<DropDownList>(*box));
    get_text_box(*box).get_highlight()->set(Highlight(0, 5));
    type_text(get_focus(), "b");
    flush_queries();
    REQUIRE(get_matches(*box) == std::vector<QString>{"Brown", "Beige"});
    press_key(get_focus(), Qt::Key_Down);
    press_key(get_focus(), Qt::Key_Down);
    REQUIRE(get_text(*box) == "Beige");
    press_key(get_focus(), Qt::Key_Return);
    flush_queries();
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == "Beige");
    submissions.pop_front();
    REQUIRE(box->get_submission() == "Beige");
    REQUIRE(!is_visible<DropDownList>(*box));
  }

  UI_TEST_CASE("focus_out") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    auto other = new QWidget(&window);
    other->setFocusPolicy(Qt::StrongFocus);
    auto submissions = std::deque<QString>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(submission);
    });
    activate(window);
    focus(*box);
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(submissions.empty());
    focus(*box);
    type_text(get_focus(), "be");
    flush_queries();
    REQUIRE(get_text(*box) == "beige");
    REQUIRE(is_visible<DropDownList>(*box));
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(!is_match(*box, PopUp()));
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == "Beige");
    submissions.pop_front();
    REQUIRE(box->get_submission() == "Beige");
    focus(*box);
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(submissions.empty());
    focus(*box);
    type_text(get_focus(), "z");
    flush_queries();
    REQUIRE(is_visible<EmptyState>(*box));
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(!is_visible<EmptyState>(*box));
    REQUIRE(!is_match(*box, PopUp()));
    REQUIRE(submissions.empty());
  }

  UI_TEST_CASE("escape") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    press_key(get_focus(), Qt::Key_Escape);
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(!is_match(*box, PopUp()));
    type_text(get_focus(), "ap");
    flush_queries();
    press_key(get_focus(), Qt::Key_Return);
    flush_queries();
    press_key(get_focus(), Qt::Key_Backspace);
    flush_queries();
    REQUIRE(get_text(*box) == "Appl");
    REQUIRE(is_visible<DropDownList>(*box));
    press_key(get_focus(), Qt::Key_Escape);
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(get_text(*box) == "Appl");
    press_key(get_focus(), Qt::Key_Escape);
    flush_queries();
    REQUIRE(get_text(*box) == "apple");
    REQUIRE(box->get_submission() == "Apple");
    REQUIRE(!is_visible<DropDownList>(*box));
  }

  UI_TEST_CASE("current") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    activate(window);
    box->get_current()->set("Beige");
    REQUIRE(get_text(*box) == "Beige");
    focus(*box);
    get_text_box(*box).get_highlight()->set(Highlight(0, 5));
    type_text(get_focus(), "brown");
    flush_queries();
    REQUIRE(box->get_current()->get() == "Brown");
  }

  UI_TEST_CASE("read_only") {
    auto window = QWidget();
    auto box = new ComboBox<QString>(make_query_model(), &window);
    box->set_read_only(true);
    activate(window);
    REQUIRE(box->is_read_only());
    box->get_current()->set("Amber");
    REQUIRE(get_text(*box) == "Amber");
    focus(*box);
    type_text(get_focus(), "b");
    press_key(get_focus(), Qt::Key_Down);
    flush_queries();
    REQUIRE(get_text(*box) == "Amber");
    REQUIRE(!is_visible<DropDownList>(*box));
    box->set_read_only(false);
    REQUIRE(!box->is_read_only());
    auto submissions = std::deque<QString>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(submission);
    });
    focus(*box);
    get_text_box(*box).get_highlight()->set(Highlight(0, 5));
    type_text(get_focus(), "b");
    flush_queries();
    REQUIRE(is_visible<DropDownList>(*box));
    press_key(get_focus(), Qt::Key_Return);
    flush_queries();
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == "Brown");
    submissions.pop_front();
    REQUIRE(!is_visible<DropDownList>(*box));
  }
}
