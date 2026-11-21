#include <algorithm>
#include <Beam/Queues/Queue.hpp>
#include <QEventLoop>
#include <QTimer>
#include <QtTest/qtestkeyboard.h>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/ControllerPortfolioModel.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/CanvasView/CanvasTableWidget.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
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
  UI_TEST_CASE("task_canvas") {
    initialize_resources();
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto make_entry = [&] (int value) {
      auto entry = BlotterTaskEntry();
      entry.m_task = std::make_shared<Task>(
        IntegerNode(value), DirectoryEntry(), Ref(profile));
      return entry;
    };
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    tasks->push(make_entry(1));
    tasks->push(make_entry(2));
    auto monitors = std::make_shared<ArrayListModel<BlotterTaskMonitor>>();
    auto model = std::make_shared<CompositeBlotterModel>(
      std::make_shared<LocalTextModel>("Test"),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<LocalBooleanModel>(false), tasks, monitors,
      std::make_shared<ArrayListModel<PortfolioUpdateEntry>>());
    auto window = BlotterWindow(model,
      std::make_shared<CancelKeyBindingsModel>(),
      std::make_shared<PositionKeyBindingsModel>());
    window.show();
    QApplication::processEvents();
    auto& tasks_view = find_widget<BlotterTasksView>(window);
    auto current = tasks_view.get_current();
    auto& tabs = find_widget<TabView>(window);
    tabs.setFocus();
    QTest::keyClick(window.windowHandle(), Qt::Key_Backtab,
      Qt::ControlModifier);
    auto& view = find_widget<BlotterTaskView>(tabs);
    auto& canvas = find_widget<CanvasTableWidget>(view);
    REQUIRE(canvas.GetRoots().empty());
    current->set(TableIndex(0, 1));
    auto value = [&] {
      auto roots = canvas.GetRoots();
      REQUIRE(roots.size() == 1);
      return dynamic_cast<const IntegerNode&>(*roots.front()).GetValue();
    };
    REQUIRE(value() == 1);
    SUBCASE("current_row") {
      auto& table = find_widget<TableView>(tasks_view);
      table.get_selection()->get_row_selection()->push(1);
      REQUIRE(value() == 1);
      current->set(TableIndex(1, 1));
      REQUIRE(value() == 2);
      auto root = canvas.GetRoots().front();
      current->set(TableIndex(1, 2));
      REQUIRE(canvas.GetRoots().front() == root);
      auto& sorted =
        dynamic_cast<SortedTableModel&>(*table.get_body().get_table());
      sorted.set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
      REQUIRE(value() == 2);
      table.get_body().get_current()->set(TableIndex(1, 1));
      REQUIRE(current->get()->m_row == 0);
      REQUIRE(value() == 1);
      current->set(boost::none);
      REQUIRE(canvas.GetRoots().empty());
    }
    SUBCASE("task_changes") {
      tasks->insert(make_entry(3), 0);
      REQUIRE(current->get()->m_row == 1);
      REQUIRE(value() == 1);
      tasks->move(1, 2);
      REQUIRE(current->get()->m_row == 2);
      REQUIRE(value() == 1);
      tasks->remove(0);
      REQUIRE(current->get()->m_row == 1);
      REQUIRE(value() == 1);
      tasks->set(1, make_entry(4));
      REQUIRE(value() == 4);
      tasks->remove(1);
      REQUIRE(!current->get());
      REQUIRE(canvas.GetRoots().empty());
    }
    SUBCASE("monitor_changes") {
      monitors->push(BlotterTaskMonitor("Custom", nullptr));
      REQUIRE(value() == 1);
      auto& table = find_widget<TableView>(tasks_view);
      table.get_current()->set(TableIndex(1, 4));
      REQUIRE(value() == 2);
      monitors->remove(0);
      REQUIRE(current->get()->m_row == 1);
      REQUIRE(current->get()->m_column == 3);
      REQUIRE(value() == 2);
      tasks->remove(0);
      REQUIRE(current->get()->m_row == 0);
      REQUIRE(value() == 2);
      find_widget<TableView>(tasks_view).get_current()->set(boost::none);
      REQUIRE(canvas.GetRoots().empty());
    }
    SUBCASE("read_only") {
      auto commands = 0;
      canvas.ConnectCommandSignal([&] (auto command) { ++commands; });
      auto& node = *canvas.GetRoots().front();
      canvas.SetCurrent(node);
      canvas.setFocus();
      canvas.Edit(node);
      QTest::keyClick(window.windowHandle(), Qt::Key_9);
      QTest::keyClick(window.windowHandle(), Qt::Key_Delete);
      REQUIRE(commands == 0);
      REQUIRE(value() == 1);
      REQUIRE_FALSE(canvas.cellWidget(1, 0));
      REQUIRE_FALSE(canvas.dragEnabled());
      REQUIRE_FALSE(canvas.viewport()->acceptDrops());
      REQUIRE(canvas.contextMenuPolicy() == Qt::PreventContextMenu);
    }
  }

  UI_TEST_CASE("cancel_key_bindings") {
    using Operation = CancelKeyBindingsModel::Operation;
    initialize_resources();
    auto model = std::make_shared<CompositeBlotterModel>(
      std::make_shared<LocalTextModel>("Test"),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<ArrayListModel<BlotterTaskEntry>>(),
      std::make_shared<ArrayListModel<BlotterTaskMonitor>>(),
      std::make_shared<ArrayListModel<PortfolioUpdateEntry>>());
    auto bindings = std::make_shared<CancelKeyBindingsModel>();
    bindings->get_binding(Operation::OLDEST)->set(Qt::Key_Escape);
    auto window = BlotterWindow(
      model, bindings, std::make_shared<PositionKeyBindingsModel>());
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

  UI_TEST_CASE("positions") {
    using Operation = PositionKeyBindingsModel::Operation;
    initialize_resources();
    auto environment = TestEnvironment();
    auto clients = TestClients(Ref(environment));
    auto ticker = parse_ticker("RY.TSX");
    auto second = parse_ticker("TD.TSX");
    auto timestamp = time_from_string("2026-11-21 10:00:00");
    for(auto& symbol : {ticker, second}) {
      environment.publish(symbol, BboQuote(
        make_bid(Money(12), 100), make_ask(Money(13), 100), timestamp));
    }
    auto inventories = std::vector{
      Inventory(Position(ticker, Currencies::CAD, 100, Money(1000)),
        Money::ZERO, Money::ZERO, 100, 1),
      Inventory(Position(second, Currencies::CAD, -50, Money(-500)),
        Money::ZERO, Money::ZERO, 50, 1)};
    auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto portfolio = std::make_shared<ControllerPortfolioModel>(
      std::make_shared<ControllerPortfolioModel::Controller>(
        Portfolio(TrueAverageBookkeeper(inventories)),
        clients.get_market_data_client(), orders));
    auto model = std::make_shared<CompositeBlotterModel>(
      std::make_shared<LocalTextModel>("Positions"),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<ArrayListModel<BlotterTaskEntry>>(),
      std::make_shared<ArrayListModel<BlotterTaskMonitor>>(), portfolio);
    auto bindings = std::make_shared<PositionKeyBindingsModel>();
    bindings->get_binding(Operation::FLATTEN)->set(Qt::CTRL | Qt::Key_G);
    auto window = BlotterWindow(
      model, std::make_shared<CancelKeyBindingsModel>(), bindings);
    REQUIRE(window.get_model()->get_portfolio() == portfolio);
    auto& view = find_widget<BlotterPositionsView>(window);
    REQUIRE(view.get_portfolio() == portfolio);
    auto& table = find_widget<TableView>(view);
    REQUIRE(table.get_table()->get_row_size() == 2);
    window.show();
    QApplication::processEvents();
    table.get_body().setFocus();
    auto requests = std::vector<std::vector<Position>>();
    auto connection = boost::signals2::scoped_connection(
      window.connect_flatten_positions_signal([&] (const auto& positions) {
        requests.push_back(positions);
      }));
    auto handle = window.windowHandle();
    QTest::keyClick(handle, Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(requests.size() == 1);
    REQUIRE(std::ranges::is_permutation(requests.back(),
      std::vector{inventories[0].m_position, inventories[1].m_position}));
    auto row = 0;
    while(table.get_table()->get<Ticker>(row, 0) != ticker) {
      ++row;
    }
    auto selection = table.get_selection()->get_row_selection();
    clear(*selection);
    selection->push(row);
    QTest::keyClick(handle, Qt::Key_F, Qt::ControlModifier);
    REQUIRE(requests.size() == 1);
    QTest::keyClick(handle, Qt::Key_G, Qt::ControlModifier);
    REQUIRE(requests.size() == 2);
    REQUIRE(requests.back() == std::vector{inventories[0].m_position});
    auto& tasks = find_widget<BlotterTasksView>(window);
    find_widget<TableView>(tasks).get_body().setFocus();
    QTest::keyClick(handle, Qt::Key_G, Qt::ControlModifier);
    REQUIRE(requests.size() == 2);
    table.get_body().setFocus();
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(ticker, Currencies::CAD, Side::ASK,
        "TSX", 100, Money(11)), 1, timestamp));
    auto wait_for = [&] (const auto& action, const auto& condition) {
      auto loop = QEventLoop();
      auto connection = boost::signals2::scoped_connection(
        portfolio->connect_operation_signal([&] (const auto&) {
          if(condition()) {
            loop.quit();
          }
        }));
      QTimer::singleShot(5000, &loop, &QEventLoop::quit);
      action();
      if(!condition()) {
        loop.exec();
      }
      REQUIRE(condition());
    };
    wait_for([&] {
      orders->push(order);
      Nexus::Tests::accept(*order);
      Nexus::Tests::fill(*order, Money(11), 40);
    }, [&] {
      return table.get_table()->get<Quantity>(row, 1) == 60;
    });
    QTest::keyClick(handle, Qt::Key_G, Qt::ControlModifier);
    REQUIRE(requests.size() == 3);
    REQUIRE(requests.back() ==
      std::vector{Position(ticker, Currencies::CAD, 60, Money(600))});
    wait_for([&] {
      Nexus::Tests::fill(*order, Money(11), 60);
    }, [&] {
      return table.get_table()->get_row_size() == 1;
    });
    REQUIRE(selection->get_size() == 0);
    QTest::keyClick(handle, Qt::Key_G, Qt::ControlModifier);
    REQUIRE(requests.size() == 3);
    bindings->get_binding(Operation::FLATTEN_ALL)->set(
      Qt::CTRL | Qt::SHIFT | Qt::Key_G);
    QTest::keyClick(handle, Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(requests.size() == 3);
    QTest::keyClick(handle, Qt::Key_G, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(requests.size() == 4);
    REQUIRE(requests.back() == std::vector{inventories[1].m_position});
  }
}
