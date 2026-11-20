#include <QtTest/qtestkeyboard.h>
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;

namespace {
  template<typename T>
  T& find_widget(QWidget& parent) {
    for(auto child : parent.findChildren<QWidget*>()) {
      if(auto widget = dynamic_cast<T*>(child)) {
        return *widget;
      }
    }
    throw std::runtime_error("Widget not found.");
  }
}

TEST_SUITE("BlotterWindow") {
  UI_TEST_CASE("cancel_key_bindings") {
    using Operation = CancelKeyBindingsModel::Operation;
    initialize_resources();
    auto model = std::make_shared<CompositeBlotterModel>(
      std::make_shared<LocalTextModel>("Test"),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<ArrayListModel<BlotterTaskEntry>>(),
      std::make_shared<ArrayListModel<BlotterTaskMonitor>>());
    auto bindings = std::make_shared<CancelKeyBindingsModel>();
    bindings->get_binding(Operation::OLDEST)->set(Qt::Key_Escape);
    auto window = BlotterWindow(model, bindings);
    auto task_requests = std::vector<Operation>();
    window.connect_cancel_task_operation_signal([&] (auto operation) {
      task_requests.push_back(operation);
    });
    auto order_requests = std::vector<Operation>();
    window.connect_cancel_order_operation_signal([&] (auto operation) {
      order_requests.push_back(operation);
    });
    window.show();
    QApplication::processEvents();
    auto handle = window.windowHandle();
    auto& tasks = find_widget<BlotterTasksView>(window);
    auto& body = find_widget<TableView>(tasks).get_body();
    body.setFocus();
    QTest::keyClick(handle, Qt::Key_Escape);
    REQUIRE(task_requests == std::vector{Operation::OLDEST});
    REQUIRE(order_requests.empty());
    auto& tabs = find_widget<TabView>(window);
    tabs.setFocus();
    QTest::keyClick(handle, Qt::Key_Escape);
    REQUIRE(task_requests.size() == 1);
    REQUIRE(order_requests.empty());
    QTest::keyClick(handle, Qt::Key_Tab, Qt::ControlModifier);
    QTest::keyClick(handle, Qt::Key_Tab, Qt::ControlModifier);
    auto& orders = find_widget<OrderLogView>(tabs);
    auto& order_body = find_widget<TableView>(orders).get_body();
    order_body.setFocus();
    bindings->get_binding(Operation::OLDEST)->set(Qt::CTRL | Qt::Key_F2);
    QTest::keyClick(handle, Qt::Key_Escape);
    REQUIRE(task_requests.size() == 1);
    REQUIRE(order_requests.empty());
    QTest::keyClick(handle, Qt::Key_F2, Qt::ControlModifier);
    REQUIRE(task_requests.size() == 1);
    REQUIRE(order_requests == std::vector{Operation::OLDEST});
  }
}
