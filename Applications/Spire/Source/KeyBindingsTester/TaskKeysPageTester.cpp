#include <algorithm>
#include <QKeyEvent>
#include "Spire/KeyBindings/OrderTaskArgumentsTableModel.hpp"
#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/LineInputForm.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  T& find_child(QWidget& parent) {
    for(auto child : parent.findChildren<QWidget*>()) {
      if(auto widget = dynamic_cast<T*>(child)) {
        return *widget;
      }
    }
    throw std::runtime_error("Child widget not found.");
  }
}

TEST_SUITE("TaskKeysPage") {
  UI_TEST_CASE("reassign_task_key") {
    auto bindings = std::make_shared<KeyBindingsModel>();
    auto& tasks = *bindings->get_order_task_arguments();
    auto key = QKeySequence(Qt::Key_F3);
    auto task = OrderTaskArguments();
    task.m_name = "First";
    task.m_scope = Scope(Venues::TSX);
    task.m_key = key;
    tasks.push(task);
    task.m_name = "Other venue";
    task.m_scope = Scope(Venues::ASX);
    tasks.push(task);
    task.m_name = "Second F3";
    task.m_scope = Scope(Venues::TSX);
    task.m_key = QKeySequence();
    tasks.push(task);
    auto page =
      TaskKeysPage(bindings, std::make_shared<LocalQueryModel<TickerInfo>>(),
        get_default_additional_tag_database());
    auto& table = *find_child<TableView>(page).get_table();
    auto row = 2;
    SUBCASE("visible_binding") {}
    SUBCASE("hidden_binding") {
      find_child<SearchBox>(page).get_current()->set("Second");
      REQUIRE(table.get_row_size() == 1);
      row = 0;
    }
    SUBCASE("previous_binding_leaves_search_results") {
      find_child<SearchBox>(page).get_current()->set("F3");
      REQUIRE(table.get_row_size() == 3);
    }
    auto column = 1 + static_cast<int>(OrderTaskColumns::KEY);
    REQUIRE(table.set(row, column, key) == QValidator::Acceptable);
    REQUIRE(tasks.get(2).m_key == key);
    REQUIRE(tasks.get(0).m_key.isEmpty());
    REQUIRE(tasks.get(1).m_key == key);
    find_child<SearchBox>(page).get_current()->set("Second");
    auto scope_column = 1 + static_cast<int>(OrderTaskColumns::SCOPE);
    REQUIRE(table.set(0, scope_column, Scope(Venues::ASX)) ==
      QValidator::Acceptable);
    REQUIRE(tasks.get(1).m_key.isEmpty());
    REQUIRE(tasks.get(2).m_key == key);
  }

  UI_TEST_CASE("search_result_actions") {
    auto bindings = std::make_shared<KeyBindingsModel>();
    auto& tasks = *bindings->get_order_task_arguments();
    auto hidden = OrderTaskArguments();
    hidden.m_name = "Hidden";
    tasks.push(hidden);
    auto match = OrderTaskArguments();
    match.m_name = "Match";
    match.m_key = QKeySequence(Qt::Key_F3);
    tasks.push(match);
    auto page = TaskKeysPage(bindings,
      std::make_shared<LocalQueryModel<TickerInfo>>(),
      get_default_additional_tag_database());
    auto& search = find_child<SearchBox>(page);
    auto& table = find_child<TableView>(page);
    search.get_current()->set("Match");
    REQUIRE(table.get_table()->get_row_size() == 1);
    table.get_current()->set(TableIndex(0, 1));
    table.get_selection()->get_row_selection()->push(0);
    auto children = page.findChildren<QWidget*>();
    auto button = std::ranges::find_if(children, [] (auto widget) {
      return widget->accessibleName() == "Delete";
    });
    REQUIRE(button != children.end());
    auto delete_task = [&] {
      auto event = QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
      QApplication::sendEvent(*button, &event);
    };
    auto duplicate_task = [&] {
      auto event = QKeyEvent(QEvent::KeyPress, Qt::Key_D, Qt::ControlModifier);
      QApplication::sendEvent(&page, &event);
    };
    SUBCASE("new_task") {
      auto create = [&] (const QString& name) {
        auto open =
          QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::ShiftModifier);
        QApplication::sendEvent(&page, &open);
        auto& form = find_child<LineInputForm>(page);
        auto& input = dynamic_cast<TextBox&>(*form.focusProxy());
        input.get_current()->set(name);
        auto submit =
          QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(&find_child<QLineEdit>(input), &submit);
        QApplication::processEvents();
      };
      SUBCASE("before_current") {
        create("Match copy");
        REQUIRE(tasks.get_size() == 3);
        REQUIRE(tasks.get(0).m_name == hidden.m_name);
        REQUIRE(tasks.get(1).m_name == "Match copy");
        REQUIRE(tasks.get(2).m_key == match.m_key);
        REQUIRE((table.get_current()->get() == TableIndex(0, 1)));
        delete_task();
        REQUIRE(tasks.get_size() == 2);
        REQUIRE(tasks.get(1).m_key == match.m_key);
      }
      SUBCASE("without_current") {
        table.get_current()->set(boost::none);
        create("Match copy");
        REQUIRE(tasks.get_size() == 3);
        REQUIRE(tasks.get(2).m_name == "Match copy");
        REQUIRE((table.get_current()->get() == TableIndex(1, 1)));
        REQUIRE(table.get_selection()->get_row_selection()->get(0) == 1);
      }
      SUBCASE("hidden_by_search") {
        create("Other");
        REQUIRE(tasks.get_size() == 3);
        REQUIRE(tasks.get(1).m_name == "Other");
        REQUIRE(table.get_table()->get_row_size() == 1);
        REQUIRE(table.get_selection()->get_row_selection()->get(0) == 0);
        REQUIRE((table.get_current()->get() == TableIndex(0, 1)));
      }
    }
    SUBCASE("duplicate") {
      duplicate_task();
      REQUIRE(tasks.get_size() == 3);
      REQUIRE(tasks.get(0).m_name == hidden.m_name);
      REQUIRE(tasks.get(1).m_name == match.m_name);
      REQUIRE(tasks.get(1).m_key.isEmpty());
      REQUIRE(tasks.get(2).m_key == match.m_key);
      REQUIRE(table.get_current()->get().has_value());
      REQUIRE(*table.get_current()->get() == TableIndex(0, 1));
      delete_task();
      REQUIRE(tasks.get_size() == 2);
      REQUIRE(tasks.get(1).m_key == match.m_key);
    }
    SUBCASE("duplicate_hidden_by_search") {
      search.get_current()->set("F3");
      duplicate_task();
      REQUIRE(tasks.get_size() == 3);
      REQUIRE(table.get_table()->get_row_size() == 1);
      REQUIRE(table.get_selection()->get_row_selection()->get_size() == 0);
      REQUIRE(!table.get_current()->get());
      REQUIRE(!(*button)->isEnabled());
    }
    SUBCASE("delete") {
      delete_task();
      REQUIRE(tasks.get_size() == 1);
      REQUIRE(tasks.get(0).m_name == hidden.m_name);
      REQUIRE(table.get_table()->get_row_size() == 0);
    }
  }
}
