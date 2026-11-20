#include <algorithm>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;

    Fixture()
      : m_profile("", false, false, {}, {},
          get_default_additional_tag_database(), {}, {}, {},
          Clients(std::in_place_type<TestClients>, Ref(m_environment))) {}

    auto make_entry() {
      auto node = CustomNode(
        "Task", {CustomNode::Child("quantity", IntegerType::GetInstance())});
      auto task = node.Replace(
        node.GetChildren().front(), std::make_unique<IntegerNode>(7));
      auto entry = BlotterTaskEntry();
      entry.m_task =
        std::make_shared<Task>(*task, DirectoryEntry(), Ref(m_profile));
      return entry;
    }
  };
}

TEST_SUITE("MonitoredTasksModel") {
  UI_TEST_CASE("ready_task_values") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("MRU.TSX");
    auto builder = CanvasNodeBuilder(SingleOrderTaskNode());
    builder.Replace(SingleOrderTaskNode::TICKER_PROPERTY,
      std::make_unique<TickerNode>(ticker));
    builder.Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(Side::ASK));
    builder.Replace(SingleOrderTaskNode::PRICE_PROPERTY,
      std::make_unique<MoneyNode>(Money(15)));
    builder.Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
      std::make_unique<IntegerNode>(75));
    auto entry = BlotterTaskEntry();
    entry.m_task = std::make_shared<Task>(
      *builder.Make(), DirectoryEntry(), Ref(fixture.m_profile));
    auto monitors = make_default_task_monitors();
    monitors->push(BlotterTaskMonitor(
      "Custom", std::make_shared<ReferenceNode>("target.quantity")));
    auto tasks = MonitoredTasksModel(monitors);
    tasks.push(entry);
    auto& row = tasks.get(0);
    REQUIRE(row.m_state == Task::State::READY);
    auto value = [&] (int column) -> const std::any& {
      return row.m_monitors.at(column)->get();
    };
    REQUIRE(std::any_cast<Ticker>(value(0)) == ticker);
    REQUIRE(std::any_cast<Side>(value(1)) == Side::ASK);
    REQUIRE(std::any_cast<Money>(value(2)) == Money(15));
    REQUIRE(std::any_cast<Quantity>(value(3)) == 75);
    REQUIRE(!value(4).has_value());
    REQUIRE(std::any_cast<Quantity>(value(5)) == 75);
  }

  UI_TEST_CASE("task_monitors") {
    auto fixture = Fixture();
    auto monitors = std::make_shared<ArrayListModel<BlotterTaskMonitor>>();
    auto tasks = MonitoredTasksModel(monitors);
    auto first = fixture.make_entry();
    tasks.push(first);
    auto defaults = make_default_task_monitors();
    auto quantity = std::ranges::find(std::as_const(*defaults),
      QString("Quantity"), &BlotterTaskMonitor::m_name);
    REQUIRE(quantity != defaults->cend());
    auto monitor = *quantity;
    monitors->push(monitor);
    auto observer = tasks.get(0).m_monitors.at(0);
    auto next = fixture.make_entry();
    tasks.insert(next, 0);
    REQUIRE(tasks.get(0).m_monitors.at(0) != observer);
    REQUIRE(tasks.get(1).m_monitors.at(0) == observer);
    REQUIRE(observer->test(Quantity(9)) == QValidator::Invalid);
    REQUIRE(observer->set(Quantity(9)) == QValidator::Invalid);
    REQUIRE(std::any_cast<Quantity>(observer->get()) == 7);
    first.m_is_pinned = true;
    tasks.set(1, first);
    REQUIRE(tasks.get(1).m_monitors.at(0) == observer);
    monitor.m_name = "Renamed";
    monitors->set(0, monitor);
    REQUIRE(tasks.get(1).m_monitors.at(0) == observer);
    monitors->insert(
      BlotterTaskMonitor("Second", std::make_shared<IntegerNode>(2)), 0);
    auto second = tasks.get(1).m_monitors.at(0);
    monitors->move(1, 0);
    REQUIRE(tasks.get(1).m_monitors.at(0) == observer);
    REQUIRE(tasks.get(1).m_monitors.at(1) == second);
    monitor.m_expression.reset();
    monitors->set(0, monitor);
    REQUIRE_FALSE(tasks.get(1).m_monitors.at(0));
    observer.reset();
    monitor.m_expression = std::make_shared<IntegerNode>(3);
    monitors->set(0, monitor);
    REQUIRE(std::any_cast<Quantity>(tasks.get(1).m_monitors.at(0)->get()) == 3);
    REQUIRE(tasks.get(1).m_monitors.at(1) == second);
    monitors->remove(0);
    REQUIRE(tasks.get(1).m_monitors.size() == 1);
    REQUIRE(tasks.get(1).m_monitors.front() == second);
    tasks.set(1, fixture.make_entry());
    REQUIRE(tasks.get(1).m_monitors.front() != second);
    monitors->remove(0);
    REQUIRE(tasks.get(0).m_monitors.empty());
    REQUIRE(tasks.get(1).m_monitors.empty());
  }
}
