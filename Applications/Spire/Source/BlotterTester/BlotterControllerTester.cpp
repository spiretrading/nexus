#include <boost/optional/optional_io.hpp>
#include <QEvent>
#include <QEventLoop>
#include <QKeyEvent>
#include <QMenu>
#include <QPointer>
#include <QTableView>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;
    std::shared_ptr<BlotterCollection> m_collection;
    FlattenController m_flatten_controller;
    std::shared_ptr<TasksModel> m_tasks;
    std::shared_ptr<ArrayListModel<std::shared_ptr<Order>>> m_orders;
    std::shared_ptr<ArrayListModel<PortfolioUpdateEntry>> m_portfolio;
    std::shared_ptr<CancelKeyBindingsModel> m_cancel_bindings;
    std::shared_ptr<PositionKeyBindingsModel> m_position_bindings;
    std::shared_ptr<Queue<std::shared_ptr<Order>>> m_submissions;
    std::shared_ptr<BlotterModel> m_model;

    Fixture()
      : Fixture(false) {}

    explicit Fixture(bool is_consolidated)
      : Fixture(is_consolidated, {}) {}

    Fixture(bool is_consolidated, const std::vector<ExchangeRate>& rates)
        : m_profile("", false, false, rates, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<TestClients>, Ref(m_environment))),
          m_collection(std::make_shared<BlotterCollection>(
            m_profile.GetClients().get_market_data_client())),
          m_flatten_controller(Ref(m_profile), m_collection),
          m_tasks(std::make_shared<ArrayListModel<BlotterTaskEntry>>()),
          m_orders(std::make_shared<ArrayListModel<std::shared_ptr<Order>>>()),
          m_portfolio(std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()),
          m_cancel_bindings(std::make_shared<CancelKeyBindingsModel>()),
          m_position_bindings(std::make_shared<PositionKeyBindingsModel>()),
          m_submissions(std::make_shared<Queue<std::shared_ptr<Order>>>()) {
      initialize_resources();
      m_environment.monitor_order_submissions(m_submissions);
      m_model = m_collection->add(std::make_shared<CompositeBlotterModel>(
        m_profile.GetClients().get_service_locator_client().get_account(),
        is_consolidated, std::make_shared<LocalTextModel>("Test"),
        std::make_shared<LocalBooleanModel>(false),
        std::make_shared<LocalBooleanModel>(false), m_tasks,
        std::make_shared<ArrayListModel<BlotterTaskMonitor>>(),
        m_orders, m_portfolio));
    }

    ~Fixture() {
      for(auto& entry : std::as_const(*m_tasks)) {
        entry.m_task->GetExecutor().Close();
      }
      flush_pending_routines();
    }

    auto make_controller() {
      return std::make_unique<BlotterController>(m_model, m_tasks, m_collection,
        m_cancel_bindings, m_position_bindings,
        Ref(m_flatten_controller), Ref(m_profile));
    }

    auto submit(Side side, Money price) {
      auto order = m_profile.GetClients().get_order_execution_client().submit(
        make_limit_order_fields(m_model->get_account(), parse_ticker("RY.TSX"),
          Currencies::CAD, side, "TSX", 100, price));
      auto submission = m_submissions->pop();
      m_environment.accept(*submission);
      m_orders->push(order);
      return submission;
    }

    void trade(const Position& position) {
      if(position.m_quantity == 0) {
        return;
      }
      auto order = m_profile.GetClients().get_order_execution_client().submit(
        make_limit_order_fields(m_model->get_account(), position.m_ticker,
          position.m_currency, get_side(position), "TSX",
          abs(position.m_quantity), Money(10)));
      auto submission = m_submissions->pop();
      m_environment.fill(*submission, abs(position.m_quantity));
      m_orders->push(order);
    }
  };

  template<typename T>
  T& find_widget(QWidget& parent) {
    for(auto widget : parent.findChildren<QWidget*>()) {
      if(auto result = dynamic_cast<T*>(widget)) {
        return *result;
      }
    }
    throw std::runtime_error("Widget not found.");
  }

  void key(QWidget& widget, Qt::Key key, Qt::KeyboardModifiers modifiers) {
    auto event = QKeyEvent(QEvent::KeyPress, key, modifiers);
    QApplication::sendEvent(&widget, &event);
    QApplication::processEvents();
  }

  void wait_for(auto& model, const auto& action, const auto& condition) {
    auto loop = QEventLoop();
    auto connection = scoped_connection(
      model.connect_operation_signal([&] (const auto&) {
        if(condition()) {
          loop.quit();
        }
      }));
    action();
    if(!condition()) {
      loop.exec();
    }
    REQUIRE(condition());
  }

  auto get_status(const Order& order) {
    return order.get_publisher().with([] (const auto& reports) {
      return reports->back().m_status;
    });
  }

  void wait_for_order(Task& task) {
    auto loop = QEventLoop();
    auto events = QtTaskQueue();
    auto order = std::shared_ptr<Order>();
    task.GetContext().GetOrderPublisher().monitor(
      events.get_slot<std::shared_ptr<Order>>([&] (const auto& update) {
        order = update;
        loop.quit();
      }));
    task.GetPublisher().monitor(
      events.get_slot<Task::StateEntry>([&] (const auto& update) {
        if(IsTerminal(update.m_state)) {
          loop.quit();
        }
      }));
    loop.exec();
    REQUIRE(order);
    order->get_publisher().with([] (const auto& reports) {
      for(auto& report : *reports) {
        INFO(report.m_text);
        REQUIRE(report.m_status != OrderStatus::REJECTED);
      }
    });
  }

  auto make_node(const Ticker& ticker, Side side, Money price) {
    auto builder = CanvasNodeBuilder(SingleOrderTaskNode());
    builder.Replace(SingleOrderTaskNode::TICKER_PROPERTY,
      std::make_unique<TickerNode>(ticker));
    builder.Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(side));
    builder.Replace(SingleOrderTaskNode::PRICE_PROPERTY,
      std::make_unique<MoneyNode>(price));
    return builder.Make();
  }
}

TEST_SUITE("BlotterController") {
  UI_TEST_CASE("portfolio_summary") {
    auto fixture = Fixture(true,
      {ExchangeRate(CurrencyPair(Currencies::CAD, Currencies::USD),
        rational<int>(2, 1))});
    auto entry = PortfolioUpdateEntry();
    entry.m_inventory = Inventory(
      Position(parse_ticker("RY.TSX"), Currencies::CAD, 100, Money(1000)),
      Money(20), Money(3), 100, 1);
    entry.m_unrealized = Money(5);
    fixture.m_portfolio->push(entry);
    auto controller = fixture.make_controller();
    auto find_values = [&] {
      auto& bar = find_widget<BlotterStatusBar>(controller->open());
      auto values = std::vector<MoneyBox*>();
      for(auto child : bar.findChildren<QWidget*>()) {
        if(auto value = dynamic_cast<MoneyBox*>(child)) {
          values.push_back(value);
        }
      }
      REQUIRE(values.size() == 7);
      return values;
    };
    auto values = find_values();
    auto& total = *values[0];
    auto set_parameters = [&] (CurrencyId currency, Money buying_power,
        Money net_loss, Money expected) {
      auto loop = QEventLoop();
      auto is_updated = [&] {
        return total.get_current()->get() == expected &&
          values[5]->get_current()->get() == buying_power &&
          values[6]->get_current()->get() == net_loss;
      };
      auto connections = ConnectionGroup();
      for(auto index : {0, 5, 6}) {
        connections.add(values[index]->get_current()->connect_update_signal(
          [&] (const auto&) {
            if(is_updated()) {
              loop.quit();
            }
          }));
      }
      auto parameters = RiskParameters(
        currency, buying_power, RiskState::Type::ACTIVE, net_loss,
        seconds(10));
      fixture.m_profile.GetClients().get_administration_client().submit(
        fixture.m_model->get_account(), RiskModification(parameters),
        ptime(), Nexus::Message());
      if(!is_updated()) {
        loop.exec();
      }
      REQUIRE(total.get_current()->get() == expected);
      REQUIRE(values[5]->get_current()->get() == buying_power);
      REQUIRE(values[6]->get_current()->get() == net_loss);
    };
    set_parameters(Currencies::CAD, Money(100000), Money(1000), Money(22));
    set_parameters(Currencies::USD, Money(200000), Money(2000), Money(44));
    set_parameters(Currencies::USD, Money(300000), Money(3000), Money(44));
    entry.m_unrealized = Money(-25);
    fixture.m_portfolio->set(0, entry);
    REQUIRE(total.get_current()->get() == Money(-16));
    controller->close();
    entry.m_unrealized = Money(10);
    fixture.m_portfolio->set(0, entry);
    values = find_values();
    REQUIRE(values[0]->get_current()->get() == Money(54));
    REQUIRE(values[5]->get_current()->get() == Money(300000));
    REQUIRE(values[6]->get_current()->get() == Money(3000));
  }

  UI_TEST_CASE("add_and_observe") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto task = controller->add(IntegerNode(1));
    REQUIRE_FALSE(controller->is_open()->get());
    REQUIRE(fixture.m_tasks->get_size() == 1);
    REQUIRE(fixture.m_tasks->get(0).m_task == task);
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::READY);
    REQUIRE(task->GetContext().GetExecutingAccount() ==
      fixture.m_model->get_account());
    auto visible = fixture.m_model->get_tasks();
    auto entry = visible->get(0);
    entry.m_is_pinned = true;
    visible->set(0, entry);
    task->Cancel();
    QApplication::processEvents();
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::CANCELED);
    REQUIRE(visible->get(0).m_state == Task::State::CANCELED);
    REQUIRE(visible->get(0).m_is_pinned);
  }

  UI_TEST_CASE("submit_without_window") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto loop = QEventLoop();
    auto connection = scoped_connection(
      fixture.m_tasks->connect_operation_signal([&] (const auto&) {
        if(fixture.m_tasks->get_size() == 1 &&
            IsTerminal(fixture.m_tasks->get(0).m_state)) {
          loop.quit();
        }
      }));
    auto task = controller->submit(IntegerNode(1));
    if(!IsTerminal(fixture.m_tasks->get(0).m_state)) {
      loop.exec();
    }
    REQUIRE_FALSE(controller->is_open()->get());
    REQUIRE(fixture.m_tasks->get(0).m_task == task);
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::COMPLETE);
  }

  UI_TEST_CASE("cancel_ticker_and_recency") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto ticker = parse_ticker("RY.TSX");
    controller->add(*make_node(ticker, Side::BID, Money(10)));
    controller->add(*make_node(ticker, Side::BID, Money(11)));
    controller->add(*make_node(parse_ticker("BMO.TSX"), Side::BID, Money(10)));
    controller->cancel(CancelKeyBindingsModel::Operation::MOST_RECENT, ticker,
      {}, none);
    QApplication::processEvents();
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::READY);
    REQUIRE(fixture.m_tasks->get(1).m_state == Task::State::CANCELED);
    REQUIRE(fixture.m_tasks->get(2).m_state == Task::State::READY);
    controller->cancel(CancelKeyBindingsModel::Operation::MOST_RECENT, ticker,
      {}, none);
    QApplication::processEvents();
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::CANCELED);
    REQUIRE(fixture.m_tasks->get(2).m_state == Task::State::READY);
  }

  UI_TEST_CASE("cancel_selection_and_prices") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto ticker = parse_ticker("RY.TSX");
    constexpr auto TASK_COUNT = 3;
    for(auto i = 0; i != TASK_COUNT; ++i) {
      auto task = controller->add(*make_node(ticker, Side::ASK, Money(10 + i)));
      task->GetContext().GetOrderPublisher().push(
        std::make_shared<PrimitiveOrder>(OrderInfo(
          make_limit_order_fields(ticker, Currencies::CAD, Side::ASK, "TSX",
            100, Money(10 + i)), i + 1,
          time_from_string("2026-11-23 10:00:00"))));
    }
    auto prices = std::unordered_map<OrderId, Money>(
      {{1, Money(20)}, {2, Money(15)}, {3, Money(5)}});
    controller->cancel(CancelKeyBindingsModel::Operation::ALL, ticker, prices, std::vector<OrderId>());
    QApplication::processEvents();
    for(auto& entry : std::as_const(*fixture.m_tasks)) {
      REQUIRE(entry.m_state == Task::State::READY);
    }
    controller->cancel(CancelKeyBindingsModel::Operation::CLOSEST_ASK,
      ticker, prices, std::vector<OrderId>{1, 2});
    QApplication::processEvents();
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::READY);
    REQUIRE(fixture.m_tasks->get(1).m_state == Task::State::CANCELED);
    REQUIRE(fixture.m_tasks->get(2).m_state == Task::State::READY);
  }

  UI_TEST_CASE("window_task_actions") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    controller->add(IntegerNode(1));
    controller->add(IntegerNode(2));
    SUBCASE("reopened") {
      controller->open();
      controller->close();
      QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    auto& window = controller->open();
    QApplication::processEvents();
    auto& view = find_widget<BlotterTasksView>(window);
    auto& table = find_widget<TableView>(view);
    auto rows = table.get_selection()->get_row_selection();
    clear(*rows);
    rows->push(0);
    table.get_body().setFocus();
    wait_for(*fixture.m_tasks, [&] {
      key(view, Qt::Key_Return, Qt::NoModifier);
    }, [&] {
      return fixture.m_tasks->get(0).m_state == Task::State::COMPLETE;
    });
    REQUIRE(fixture.m_tasks->get(1).m_state == Task::State::READY);
    clear(*rows);
    rows->push(1);
    key(view, Qt::Key_Escape, Qt::NoModifier);
    REQUIRE(fixture.m_tasks->get(1).m_state == Task::State::CANCELED);
  }

  UI_TEST_CASE("window_task_cancel_operation") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    controller->add(IntegerNode(1));
    controller->add(IntegerNode(2));
    fixture.m_cancel_bindings->get_binding(
      CancelKeyBindingsModel::Operation::MOST_RECENT)->set(Qt::Key_F8);
    auto& window = controller->open();
    QApplication::processEvents();
    auto& view = find_widget<BlotterTasksView>(window);
    find_widget<TableView>(view).get_body().setFocus();
    key(view, Qt::Key_F8, Qt::NoModifier);
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::READY);
    REQUIRE(fixture.m_tasks->get(1).m_state == Task::State::CANCELED);
    key(view, Qt::Key_F8, Qt::NoModifier);
    REQUIRE(fixture.m_tasks->get(0).m_state == Task::State::CANCELED);
  }

  UI_TEST_CASE("window_cancel_selected_orders") {
    auto fixture = Fixture();
    auto first = fixture.submit(Side::BID, Money(10));
    auto second = fixture.submit(Side::ASK, Money(11));
    auto controller = fixture.make_controller();
    auto& window = controller->open();
    find_widget<TabView>(window).get_current()->set(2);
    auto& view = find_widget<OrderLogView>(window);
    auto rows = find_widget<TableView>(view).
      get_selection()->get_row_selection();
    clear(*rows);
    rows->push(1);
    key(view, Qt::Key_Escape, Qt::NoModifier);
    flush_pending_routines();
    REQUIRE(get_status(*first) == OrderStatus::NEW);
    REQUIRE(get_status(*second) == OrderStatus::PENDING_CANCEL);
    REQUIRE(fixture.m_tasks->get_size() == 0);
  }

  UI_TEST_CASE("window_cancel_order_operations") {
    using Operation = CancelKeyBindingsModel::Operation;
    auto fixture = Fixture();
    auto orders = std::vector<std::shared_ptr<Order>>{
      fixture.submit(Side::ASK, Money(12)),
      fixture.submit(Side::BID, Money(9)),
      fixture.submit(Side::ASK, Money(11)),
      fixture.submit(Side::BID, Money(10)),
      fixture.submit(Side::ASK, Money(11)),
      fixture.submit(Side::BID, Money(10)),
      fixture.submit(Side::ASK, Money(12)),
      fixture.submit(Side::BID, Money(9))};
    auto operation = Operation::ALL;
    auto expected = std::vector<int>();
    SUBCASE("all") {
      expected = {0, 1, 2, 3, 4, 5, 6, 7};
    }
    SUBCASE("all_asks") {
      operation = Operation::ALL_ASKS;
      expected = {0, 2, 4, 6};
    }
    SUBCASE("all_bids") {
      operation = Operation::ALL_BIDS;
      expected = {1, 3, 5, 7};
    }
    SUBCASE("most_recent") {
      operation = Operation::MOST_RECENT;
      expected = {7};
    }
    SUBCASE("most_recent_ask") {
      operation = Operation::MOST_RECENT_ASK;
      expected = {6};
    }
    SUBCASE("most_recent_bid") {
      operation = Operation::MOST_RECENT_BID;
      expected = {7};
    }
    SUBCASE("oldest") {
      operation = Operation::OLDEST;
      expected = {0};
    }
    SUBCASE("oldest_ask") {
      operation = Operation::OLDEST_ASK;
      expected = {0};
    }
    SUBCASE("oldest_bid") {
      operation = Operation::OLDEST_BID;
      expected = {1};
    }
    SUBCASE("closest_ask") {
      operation = Operation::CLOSEST_ASK;
      expected = {4};
    }
    SUBCASE("closest_bid") {
      operation = Operation::CLOSEST_BID;
      expected = {5};
    }
    SUBCASE("furthest_ask") {
      operation = Operation::FURTHEST_ASK;
      expected = {0};
    }
    SUBCASE("furthest_bid") {
      operation = Operation::FURTHEST_BID;
      expected = {1};
    }
    fixture.m_cancel_bindings->get_binding(operation)->set(Qt::Key_F8);
    auto controller = fixture.make_controller();
    auto& window = controller->open();
    find_widget<TabView>(window).get_current()->set(2);
    auto& view = find_widget<OrderLogView>(window);
    key(view, Qt::Key_F8, Qt::NoModifier);
    flush_pending_routines();
    for(auto i = 0; i != static_cast<int>(orders.size()); ++i) {
      if(std::ranges::find(expected, i) != expected.end()) {
        REQUIRE(get_status(*orders[i]) == OrderStatus::PENDING_CANCEL);
      } else {
        REQUIRE(get_status(*orders[i]) == OrderStatus::NEW);
      }
    }
  }

  UI_TEST_CASE("window_cancel_skips_terminal_and_pending") {
    auto fixture = Fixture();
    auto first = fixture.submit(Side::BID, Money(10));
    auto pending = fixture.submit(Side::BID, Money(10));
    auto terminal = fixture.submit(Side::BID, Money(10));
    fixture.m_profile.GetClients().get_order_execution_client().cancel(pending);
    fixture.m_profile.GetClients().get_order_execution_client().cancel(terminal);
    fixture.m_environment.cancel(*terminal);
    fixture.m_cancel_bindings->get_binding(
      CancelKeyBindingsModel::Operation::MOST_RECENT)->set(Qt::Key_F8);
    auto controller = fixture.make_controller();
    auto& window = controller->open();
    find_widget<TabView>(window).get_current()->set(2);
    auto& view = find_widget<OrderLogView>(window);
    key(view, Qt::Key_F8, Qt::NoModifier);
    flush_pending_routines();
    REQUIRE(get_status(*first) == OrderStatus::PENDING_CANCEL);
    REQUIRE(get_status(*pending) == OrderStatus::PENDING_CANCEL);
    REQUIRE(get_status(*terminal) == OrderStatus::CANCELED);
  }

  UI_TEST_CASE("flatten") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto quantity = Quantity(100);
    auto side = Side::ASK;
    SUBCASE("short") {
      quantity = -50;
      side = Side::BID;
    }
    SUBCASE("long") {}
    fixture.trade(Position(parse_ticker("RY.TSX"), Currencies::CAD,
      quantity, quantity * Money(10)));
    controller->flatten(parse_ticker("RY.TSX"));
    wait_for_order(*fixture.m_tasks->get(0).m_task);
    auto order = fixture.m_submissions->pop();
    auto& fields = order->get_info().m_fields;
    REQUIRE(fields.m_account == fixture.m_model->get_account());
    REQUIRE(fields.m_ticker == parse_ticker("RY.TSX"));
    REQUIRE(fields.m_currency == Currencies::CAD);
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == side);
    REQUIRE(fields.m_quantity == abs(quantity));
    REQUIRE(fields.m_destination ==
      DESTINATIONS.get_preferred_destination(Venues::TSX).m_id);
    REQUIRE_FALSE(controller->is_open()->get());
    REQUIRE(fixture.m_tasks->get_size() == 1);
    controller->flatten(parse_ticker("RY.TSX"));
    REQUIRE(fixture.m_tasks->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("window_flatten_positions") {
    auto fixture = Fixture(true);
    auto positions = std::vector{
      Position(parse_ticker("RY.TSX"), Currencies::CAD, 100, Money(1000)),
      Position(parse_ticker("TD.TSX"), Currencies::CAD, -50, Money(-500)),
      Position(parse_ticker("BMO.TSX"), Currencies::CAD, 0, Money::ZERO)};
    for(auto& position : positions) {
      fixture.trade(position);
      auto entry = PortfolioUpdateEntry();
      entry.m_inventory.m_position = position;
      fixture.m_portfolio->push(entry);
    }
    auto controller = fixture.make_controller();
    auto& view = find_widget<BlotterPositionsView>(controller->open());
    auto count = 1;
    SUBCASE("selected") {
      auto rows = find_widget<TableView>(view).
        get_selection()->get_row_selection();
      clear(*rows);
      rows->push(1);
      key(view, Qt::Key_F, Qt::ControlModifier);
    }
    SUBCASE("all") {
      count = 2;
      key(view, Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    }
    REQUIRE(fixture.m_tasks->get_size() == count);
    key(view, Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    REQUIRE(fixture.m_tasks->get_size() == 2);
    for(auto i = 0; i != 2; ++i) {
      wait_for_order(*fixture.m_tasks->get(i).m_task);
      auto order = fixture.m_submissions->pop();
      REQUIRE(order->get_info().m_fields.m_type == OrderType::MARKET);
      if(count == 1 && i == 0) {
        REQUIRE(order->get_info().m_fields.m_side == Side::BID);
        REQUIRE(order->get_info().m_fields.m_quantity == 50);
      }
    }
    REQUIRE(!fixture.m_submissions->try_pop());
  }

  UI_TEST_CASE("portfolio_viewer_actions") {
    auto fixture = Fixture();
    fixture.m_environment.publish(parse_ticker("RY.TSX"),
      BboQuote(make_bid(Money(10), 100), make_ask(Money(11), 100),
        time_from_string("2026-11-23 15:00:00")));
    auto properties = PortfolioViewerProperties::GetDefault();
    properties.SetSelectingAllGroups(false);
    properties.GetSelectedGroups().insert(DirectoryEntry());
    auto viewer = PortfolioViewerWindow(Ref(fixture.m_profile), properties);
    viewer.show();
    auto table = viewer.findChild<QTableView*>("m_portfolioTableView");
    REQUIRE(table);
    auto loop = QEventLoop();
    QObject::connect(table->model(), &QAbstractItemModel::rowsInserted,
      &loop, &QEventLoop::quit);
    auto initial = fixture.submit(Side::BID, Money(10));
    fixture.m_environment.fill(*initial, 100);
    if(table->model()->rowCount() == 0) {
      loop.exec();
    }
    REQUIRE(table->model()->rowCount() == 1);
    table->selectRow(0);
    auto activate = [&] (const QString& action) {
      auto activated = false;
      QMetaObject::invokeMethod(&viewer, [&] {
        for(auto widget : QApplication::topLevelWidgets()) {
          if(auto menu = dynamic_cast<QMenu*>(widget)) {
            for(auto item : menu->actions()) {
              if(item->text() == action) {
                activated = true;
                menu->setActiveAction(item);
                auto event = QKeyEvent(
                  QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
                QApplication::sendEvent(menu, &event);
                return;
              }
            }
            menu->close();
          }
        }
      }, Qt::QueuedConnection);
      table->customContextMenuRequested(QPoint());
      REQUIRE(activated);
    };
    activate("Open Blotter");
    auto& controller =
      fixture.m_profile.GetBlotterRegistry().get_consolidated();
    REQUIRE(controller.is_open()->get());
    controller.close();
    activate("Flatten Position");
    REQUIRE_FALSE(controller.is_open()->get());
    wait_for_order(*controller.get_model()->get_tasks()->get(0).m_task);
    auto order = fixture.m_submissions->pop();
    auto& fields = order->get_info().m_fields;
    REQUIRE(fields.m_account == fixture.m_model->get_account());
    REQUIRE(fields.m_ticker == parse_ticker("RY.TSX"));
    REQUIRE(fields.m_type == OrderType::MARKET);
    REQUIRE(fields.m_side == Side::ASK);
    REQUIRE(fields.m_quantity == 100);
    REQUIRE(controller.get_model()->get_tasks()->get_size() == 1);
    activate("Flatten Position");
    REQUIRE(controller.get_model()->get_tasks()->get_size() == 1);
    REQUIRE(!fixture.m_submissions->try_pop());
    controller.get_model()->get_tasks()->get(0).m_task->GetExecutor().Close();
  }

  UI_TEST_CASE("open_close") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto updates = std::vector<bool>();
    auto connection = scoped_connection(
      controller->is_open()->connect_update_signal([&] (auto value) {
        updates.push_back(value);
      }));
    REQUIRE_FALSE(controller->is_open()->get());
    controller->close();
    REQUIRE(updates.empty());
    auto window = QPointer(&controller->open());
    REQUIRE(controller->is_open()->get());
    REQUIRE(window->isVisible());
    REQUIRE(window->get_model() == fixture.m_model);
    REQUIRE(&controller->open() == window.data());
    REQUIRE(updates == std::vector{true});
    SUBCASE("controller_close") {
      controller->close();
    }
    SUBCASE("window_close") {
      REQUIRE(window->close());
    }
    REQUIRE_FALSE(controller->is_open()->get());
    REQUIRE_FALSE(window->isVisible());
    REQUIRE(updates == std::vector{true, false});
    controller->close();
    REQUIRE(updates.size() == 2);
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
  }

  UI_TEST_CASE("reopen") {
    auto fixture = Fixture();
    fixture.m_model->is_pinned()->set(true);
    auto controller = fixture.make_controller();
    auto window = QPointer(&controller->open());
    controller->close();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
    fixture.m_model->get_name()->set("Renamed");
    auto& reopened = controller->open();
    REQUIRE(controller->is_open()->get());
    REQUIRE(controller->get_model() == fixture.m_model);
    REQUIRE(reopened.get_model() == fixture.m_model);
    REQUIRE(reopened.windowTitle().contains("Renamed"));
    REQUIRE(reopened.isVisible());
  }

  UI_TEST_CASE("remove_on_close") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto is_open = controller->is_open();
    auto connection = scoped_connection(
      is_open->connect_update_signal([&] (auto value) {
        if(!value) {
          fixture.m_collection->remove(0);
          controller.reset();
        }
      }));
    auto window = QPointer(&controller->open());
    REQUIRE(window->close());
    REQUIRE_FALSE(controller);
    REQUIRE_FALSE(is_open->get());
    REQUIRE(fixture.m_collection->get_size() == 0);
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    REQUIRE(window.isNull());
  }

  UI_TEST_CASE("destroy_open_controller") {
    auto fixture = Fixture();
    auto controller = fixture.make_controller();
    auto is_open = controller->is_open();
    auto window = QPointer(&controller->open());
    controller.reset();
    REQUIRE(window.isNull());
    REQUIRE_FALSE(is_open->get());
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}
