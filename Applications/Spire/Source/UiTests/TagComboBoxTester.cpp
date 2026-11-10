#include <deque>
#include <memory>
#include <vector>
#include <doctest/doctest.h>
#include <QKeyEvent>
#include <QWidget>
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"
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

  void press_enter(QWidget& widget) {
    auto event = QKeyEvent(
      QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QString("\r"));
    QApplication::sendEvent(&widget, &event);
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

  QString get_text(QWidget& box) {
    auto tag_box = find_child<TagBox>(box);
    REQUIRE(tag_box);
    return tag_box->get_current()->get();
  }

  std::vector<QString> to_vector(const ListModel<QString>& list) {
    auto values = std::vector<QString>();
    for(auto i = 0; i != list.get_size(); ++i) {
      values.push_back(list.get(i));
    }
    return values;
  }

  std::vector<QString> get_matches(QWidget& box) {
    auto drop_down_list = find_child<DropDownList>(box);
    REQUIRE(drop_down_list);
    auto& list = *drop_down_list->get_list_view().get_list();
    auto matches = std::vector<QString>();
    for(auto i = 0; i != list.get_size(); ++i) {
      matches.push_back(std::any_cast<QString>(list.get(i)));
    }
    return matches;
  }
}

TEST_SUITE("TagComboBox") {
  UI_TEST_CASE("unmatched_text") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    type_text(get_focus(), "zz");
    press_enter(get_focus());
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(box->get_current()->get_size() == 0);
    type_text(get_focus(), "zz");
    flush_queries();
    press_enter(get_focus());
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(box->get_current()->get_size() == 0);
    type_text(get_focus(), "a");
    flush_queries();
    press_key(get_focus(), Qt::Key_Backspace);
    flush_queries();
    REQUIRE(get_text(*box) == "a");
    REQUIRE(is_visible<DropDownList>(*box));
    press_enter(get_focus());
    REQUIRE(get_text(*box) == "a");
    REQUIRE(box->get_current()->get_size() == 0);
  }

  UI_TEST_CASE("push") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    auto submissions = std::deque<std::vector<QString>>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(to_vector(*submission));
    });
    activate(window);
    focus(*box);
    type_text(get_focus(), "ap");
    flush_queries();
    REQUIRE(get_text(*box) == "apple");
    press_enter(get_focus());
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == std::vector<QString>{"Apple"});
    submissions.pop_front();
    type_text(get_focus(), "be");
    flush_queries();
    REQUIRE(get_text(*box) == "beige");
    type_text(get_focus(), " ");
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) ==
      std::vector<QString>{"Apple", "Beige"});
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == std::vector<QString>{"Apple", "Beige"});
    submissions.pop_front();
    type_text(get_focus(), "b");
    flush_queries();
    press_key(get_focus(), Qt::Key_Backspace);
    type_text(get_focus(), " ");
    flush_queries();
    REQUIRE(get_text(*box) == "b ");
    REQUIRE(box->get_current()->get_size() == 2);
    REQUIRE(submissions.empty());
  }

  UI_TEST_CASE("exclusions") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    type_text(get_focus(), "ap");
    flush_queries();
    press_enter(get_focus());
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
    type_text(get_focus(), "a");
    flush_queries();
    REQUIRE(get_matches(*box) == std::vector<QString>{"Amber", "Almond"});
    press_key(get_focus(), Qt::Key_Backspace);
    press_key(get_focus(), Qt::Key_Backspace);
    type_text(get_focus(), "apple");
    flush_queries();
    REQUIRE(get_text(*box) == "apple");
    press_enter(get_focus());
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
    box->get_current()->remove(0);
    type_text(get_focus(), "a");
    flush_queries();
    REQUIRE(get_matches(*box) ==
      std::vector<QString>{"Amber", "Almond", "Apple"});
  }

  UI_TEST_CASE("drop_down_submit") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    auto other = new QWidget(&window);
    other->setFocusPolicy(Qt::StrongFocus);
    auto submissions = std::deque<std::vector<QString>>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(to_vector(*submission));
    });
    activate(window);
    focus(*box);
    type_text(get_focus(), "a");
    flush_queries();
    press_key(get_focus(), Qt::Key_Down);
    press_key(get_focus(), Qt::Key_Down);
    press_enter(get_focus());
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Almond"});
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(!is_visible<DropDownList>(*box));
    REQUIRE(submissions.empty());
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == std::vector<QString>{"Almond"});
    submissions.pop_front();
  }

  UI_TEST_CASE("escape") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    activate(window);
    focus(*box);
    type_text(get_focus(), "ap");
    flush_queries();
    press_enter(get_focus());
    flush_queries();
    box->get_current()->push("Beige");
    type_text(get_focus(), "zz");
    flush_queries();
    press_key(get_focus(), Qt::Key_Escape);
    flush_queries();
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
    box->get_current()->push("Brown");
    press_key(get_focus(), Qt::Key_Escape);
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
  }

  UI_TEST_CASE("focus_out") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    auto other = new QWidget(&window);
    other->setFocusPolicy(Qt::StrongFocus);
    auto submissions = std::deque<std::vector<QString>>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(to_vector(*submission));
    });
    activate(window);
    focus(*box);
    type_text(get_focus(), "be");
    flush_queries();
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Beige"});
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == std::vector<QString>{"Beige"});
    submissions.pop_front();
    focus(*box);
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(submissions.empty());
    focus(*box);
    box->get_current()->push("Brown");
    other->setFocus(Qt::MouseFocusReason);
    flush_queries();
    REQUIRE(submissions.size() == 1);
    REQUIRE(submissions.front() == std::vector<QString>{"Beige", "Brown"});
    submissions.pop_front();
  }

  UI_TEST_CASE("read_only") {
    auto window = QWidget();
    auto box = new TagComboBox<QString>(make_query_model(), &window);
    auto submissions = std::deque<std::vector<QString>>();
    box->connect_submit_signal([&] (const auto& submission) {
      submissions.push_back(to_vector(*submission));
    });
    box->set_read_only(true);
    REQUIRE(box->is_read_only());
    REQUIRE(is_match(*box, ReadOnly()));
    activate(window);
    focus(*box);
    type_text(get_focus(), "ap");
    press_enter(get_focus());
    flush_queries();
    REQUIRE(get_text(*box).isEmpty());
    REQUIRE(box->get_current()->get_size() == 0);
    REQUIRE(submissions.empty());
    box->set_read_only(false);
    REQUIRE(!box->is_read_only());
    REQUIRE(!is_match(*box, ReadOnly()));
    focus(*box);
    type_text(get_focus(), "ap");
    flush_queries();
    press_enter(get_focus());
    flush_queries();
    REQUIRE(to_vector(*box->get_current()) == std::vector<QString>{"Apple"});
    REQUIRE(submissions.size() == 1);
    submissions.pop_front();
  }
}
