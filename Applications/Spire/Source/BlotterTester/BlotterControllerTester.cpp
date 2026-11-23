#include <QEvent>
#include <QEventLoop>
#include <QPointer>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

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
    std::shared_ptr<TasksModel> m_tasks;
    std::shared_ptr<BlotterModel> m_model;

    Fixture()
        : m_profile("", false, false, {}, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<TestClients>, Ref(m_environment))),
          m_collection(std::make_shared<BlotterCollection>(
            m_profile.GetClients().get_market_data_client())),
          m_tasks(std::make_shared<ArrayListModel<BlotterTaskEntry>>()) {
      initialize_resources();
      m_model = m_collection->add(std::make_shared<CompositeBlotterModel>(
        DirectoryEntry(), false, std::make_shared<LocalTextModel>("Test"),
        std::make_shared<LocalBooleanModel>(false),
        std::make_shared<LocalBooleanModel>(false), m_tasks,
        std::make_shared<ArrayListModel<BlotterTaskMonitor>>(),
        std::make_shared<ArrayListModel<std::shared_ptr<Order>>>(),
        std::make_shared<ArrayListModel<PortfolioUpdateEntry>>()));
    }

    ~Fixture() {
      for(auto& entry : std::as_const(*m_tasks)) {
        entry.m_task->GetExecutor().Close();
      }
      flush_pending_routines();
    }

    auto make_controller() {
      return std::make_unique<BlotterController>(m_model, m_tasks, m_collection,
        std::make_shared<CancelKeyBindingsModel>(),
        std::make_shared<PositionKeyBindingsModel>(), Ref(m_profile));
    }
  };

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
