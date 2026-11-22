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
#include "Spire/Spire/UnionListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::signals2;
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
    auto replacement = tasks.get(1).m_task;
    REQUIRE(tasks.move(1, 0) == QValidator::Acceptable);
    REQUIRE(tasks.get(0).m_task == replacement);
    REQUIRE(tasks.remove(0) == QValidator::Acceptable);
    REQUIRE(tasks.get_size() == 1);
    REQUIRE(tasks.get(0).m_task == next.m_task);
  }

  UI_TEST_CASE("source_pins_and_monitors") {
    auto fixture = Fixture();
    auto source =
      std::make_shared<MonitoredTasksModel>(make_default_task_monitors());
    auto entry = fixture.make_entry();
    entry.m_is_pinned = true;
    entry.m_state = Task::State::ACTIVE;
    source->push(entry);
    auto monitors = std::make_shared<ArrayListModel<BlotterTaskMonitor>>();
    monitors->push(BlotterTaskMonitor(
      "Local", std::make_shared<IntegerNode>(11)));
    auto tasks = MonitoredTasksModel(source, monitors);
    REQUIRE(tasks.get_size() == 1);
    REQUIRE(tasks.get(0).m_task == entry.m_task);
    REQUIRE(tasks.get(0).m_state == Task::State::ACTIVE);
    REQUIRE_FALSE(tasks.get(0).m_is_pinned);
    REQUIRE(tasks.get(0).m_monitors.size() == 1);
    auto observer = tasks.get(0).m_monitors.front();
    REQUIRE(std::any_cast<Quantity>(observer->get()) == 11);
    entry.m_is_pinned = false;
    source->set(0, entry);
    auto local = tasks.get(0);
    local.m_is_pinned = true;
    REQUIRE(tasks.set(0, local) == QValidator::Acceptable);
    REQUIRE(tasks.get(0).m_is_pinned);
    REQUIRE_FALSE(source->get(0).m_is_pinned);
    entry.m_state = Task::State::COMPLETE;
    source->set(0, entry);
    REQUIRE(tasks.get(0).m_is_pinned);
    REQUIRE(tasks.get(0).m_state == Task::State::COMPLETE);
    REQUIRE(tasks.get(0).m_monitors.front() == observer);
    local = tasks.get(0);
    local.m_state = Task::State::READY;
    REQUIRE(tasks.set(0, local) == QValidator::Invalid);
    REQUIRE(tasks.get(0).m_state == Task::State::COMPLETE);
    auto replacement = fixture.make_entry();
    REQUIRE(tasks.set(0, replacement) == QValidator::Invalid);
    REQUIRE(tasks.insert(replacement, 0) == QValidator::Invalid);
    REQUIRE(tasks.push(replacement) == QValidator::Invalid);
    REQUIRE(tasks.move(0, 0) == QValidator::Invalid);
    REQUIRE(tasks.remove(0) == QValidator::Invalid);
    REQUIRE(tasks.get_size() == 1);
    monitors->set(0, BlotterTaskMonitor(
      "Local", std::make_shared<IntegerNode>(12)));
    REQUIRE(std::any_cast<Quantity>(
      tasks.get(0).m_monitors.front()->get()) == 12);
    REQUIRE(source->get(0).m_monitors.size() == 5);
    observer = tasks.get(0).m_monitors.front();
    replacement.m_is_pinned = true;
    source->set(0, replacement);
    REQUIRE(tasks.get(0).m_task == replacement.m_task);
    REQUIRE_FALSE(tasks.get(0).m_is_pinned);
    REQUIRE(tasks.get(0).m_state == Task::State::READY);
    REQUIRE(tasks.get(0).m_monitors.front() != observer);
  }

  UI_TEST_CASE("source_operations") {
    auto fixture = Fixture();
    auto source = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto tasks = MonitoredTasksModel(source, make_default_task_monitors());
    auto first = fixture.make_entry();
    auto second = fixture.make_entry();
    auto operations = std::deque<TasksModel::Operation>();
    auto connection = scoped_connection(tasks.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
        visit(operation, [&] (const TasksModel::PreRemoveOperation& remove) {
          REQUIRE(tasks.get(remove.m_index).m_task == second.m_task);
        });
      }));
    source->transact([&] {
      source->push(first);
      source->insert(second, 0);
    });
    require_list_transaction<BlotterTaskEntry>(operations,
      {TasksModel::AddOperation(0), TasksModel::AddOperation(0)});
    REQUIRE(tasks.get(0).m_task == second.m_task);
    REQUIRE(tasks.get(1).m_task == first.m_task);
    auto local = tasks.get(0);
    auto observers = local.m_monitors;
    local.m_is_pinned = true;
    tasks.set(0, local);
    operations.clear();
    source->move(0, 1);
    require_list_transaction<BlotterTaskEntry>(operations,
      {TasksModel::MoveOperation(0, 1)});
    REQUIRE(tasks.get(1).m_task == second.m_task);
    REQUIRE(tasks.get(1).m_is_pinned);
    REQUIRE(tasks.get(1).m_monitors == observers);
    operations.clear();
    source->remove(1);
    require_list_transaction<BlotterTaskEntry>(operations,
      {TasksModel::PreRemoveOperation(1), TasksModel::RemoveOperation(1)});
    REQUIRE(tasks.get_size() == 1);
    REQUIRE(tasks.get(0).m_task == first.m_task);
  }

  UI_TEST_CASE("union_sources") {
    auto fixture = Fixture();
    auto hash = [] (const auto& entry) {
      return std::hash<int>()(entry.m_task->GetId());
    };
    auto is_equal = [] (const auto& left, const auto& right) {
      return left.m_task->GetId() == right.m_task->GetId();
    };
    auto source = std::make_shared<UnionListModel<
      BlotterTaskEntry, decltype(hash), decltype(is_equal)>>();
    auto first = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto second = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto entry = fixture.make_entry();
    first->push(entry);
    second->push(entry);
    source->add_source(first);
    source->add_source(second);
    auto tasks = MonitoredTasksModel(source, make_default_task_monitors());
    auto other = MonitoredTasksModel(source, make_default_task_monitors());
    REQUIRE(tasks.get_size() == 1);
    auto local = tasks.get(0);
    auto observers = local.m_monitors;
    local.m_is_pinned = true;
    tasks.set(0, local);
    REQUIRE_FALSE(other.get(0).m_is_pinned);
    entry.m_state = Task::State::ACTIVE;
    second->set(0, entry);
    source->remove_source(first);
    REQUIRE(tasks.get_size() == 1);
    REQUIRE(tasks.get(0).m_is_pinned);
    REQUIRE(tasks.get(0).m_state == Task::State::ACTIVE);
    REQUIRE(tasks.get(0).m_monitors == observers);
    REQUIRE(other.get(0).m_state == Task::State::ACTIVE);
    REQUIRE_FALSE(other.get(0).m_is_pinned);
    source->remove_source(second);
    REQUIRE(tasks.get_size() == 0);
    REQUIRE(other.get_size() == 0);
  }
}
