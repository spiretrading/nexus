#include <deque>
#include <thread>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/TaskOrdersModel.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;

    Fixture()
      : m_profile("", false, false, {}, {},
          get_default_additional_tag_database(), BookViewProperties(),
          TimeAndSalesProperties(), {},
          Clients(std::in_place_type<TestClients>, Ref(m_environment))) {}

    auto make_task() {
      auto entry = BlotterTaskEntry();
      entry.m_task = std::make_shared<Task>(
        IntegerNode(0), DirectoryEntry(), Ref(m_profile));
      return entry;
    }
  };

  auto make_order(OrderId id) {
    return std::make_shared<PrimitiveOrder>(OrderInfo(OrderFields(), id,
      time_from_string("2026-11-16 10:00:00")));
  }

  void publish(const BlotterTaskEntry& task, const OrdersModel::Type& order) {
    task.m_task->GetContext().GetOrderPublisher().push(order);
  }
}

TEST_SUITE("TaskOrdersModel") {
  UI_TEST_CASE("empty") {
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto model = TaskOrdersModel(tasks);
    REQUIRE(model.get_tasks() == tasks);
    REQUIRE(model.get_size() == 0);
    REQUIRE_THROWS_AS(model.get(0), std::out_of_range);
  }

  UI_TEST_CASE("existing_orders") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    auto order = make_order(1);
    auto other = make_order(2);
    publish(first, order);
    publish(first, order);
    publish(second, make_order(1));
    publish(second, other);
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{first, second});
    auto model = TaskOrdersModel(tasks);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == order);
    REQUIRE(model.get(1) == other);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
  }

  UI_TEST_CASE("published_orders") {
    auto fixture = Fixture();
    auto task = fixture.make_task();
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{task});
    auto model = TaskOrdersModel(tasks);
    auto operations = std::deque<OrdersModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    auto order = make_order(1);
    auto thread = std::thread([&] {
      publish(task, order);
      publish(task, order);
    });
    thread.join();
    REQUIRE(model.get_size() == 0);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == order);
    require_list_transaction<OrdersModel::Type>(
      operations, {OrdersModel::AddOperation(0)});
  }

  UI_TEST_CASE("publication_order") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{first, second});
    auto model = TaskOrdersModel(tasks);
    auto first_order = make_order(1);
    auto second_order = make_order(2);
    auto third_order = make_order(3);
    publish(second, first_order);
    publish(first, second_order);
    publish(second, third_order);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(0) == first_order);
    REQUIRE(model.get(1) == second_order);
    REQUIRE(model.get(2) == third_order);
  }

  UI_TEST_CASE("task_addition_and_removal") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    auto shared = make_order(1);
    auto other = make_order(2);
    publish(first, shared);
    publish(second, shared);
    publish(second, other);
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto model = TaskOrdersModel(tasks);
    tasks->push(first);
    tasks->insert(second, 0);
    REQUIRE(model.get_size() == 2);
    tasks->remove(1);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == shared);
    auto operations = std::deque<OrdersModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
      if(auto removal =
          std::get_if<OrdersModel::PreRemoveOperation>(&operation)) {
        REQUIRE(model.get(removal->m_index));
      }
    });
    tasks->remove(0);
    REQUIRE(model.get_size() == 0);
    require_list_transaction<OrdersModel::Type>(operations, {
      OrdersModel::PreRemoveOperation(1), OrdersModel::RemoveOperation(1),
      OrdersModel::PreRemoveOperation(0), OrdersModel::RemoveOperation(0)});
    publish(first, make_order(3));
    publish(second, make_order(4));
    QApplication::processEvents();
    REQUIRE(model.get_size() == 0);
  }

  UI_TEST_CASE("task_reordering") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    auto third = fixture.make_task();
    auto first_order = make_order(1);
    auto second_order = make_order(2);
    auto third_order = make_order(3);
    publish(first, first_order);
    publish(second, second_order);
    publish(third, third_order);
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{first, second, third});
    auto model = TaskOrdersModel(tasks);
    tasks->move(0, 2);
    tasks->move(1, 0);
    REQUIRE(model.get(0) == first_order);
    REQUIRE(model.get(1) == second_order);
    REQUIRE(model.get(2) == third_order);
    tasks->remove(0);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0) == first_order);
    REQUIRE(model.get(1) == second_order);
    auto next = make_order(4);
    publish(first, next);
    publish(third, make_order(5));
    QApplication::processEvents();
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get(2) == next);
  }

  UI_TEST_CASE("task_metadata_update") {
    auto fixture = Fixture();
    auto task = fixture.make_task();
    auto order = make_order(1);
    publish(task, order);
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{task});
    auto model = TaskOrdersModel(tasks);
    auto operations = std::deque<OrdersModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    task.m_is_pinned = true;
    task.m_state = Task::State::COMPLETE;
    tasks->set(0, task);
    QApplication::processEvents();
    REQUIRE(operations.empty());
    REQUIRE(model.get(0) == order);
  }

  UI_TEST_CASE("task_replacement") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    auto shared = make_order(1);
    auto other = make_order(2);
    publish(first, shared);
    publish(first, other);
    publish(second, shared);
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{first});
    auto model = TaskOrdersModel(tasks);
    publish(first, make_order(3));
    tasks->set(0, second);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0) == shared);
    auto next = make_order(4);
    publish(second, next);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(1) == next);
  }

  UI_TEST_CASE("duplicate_task_and_readdition") {
    auto fixture = Fixture();
    auto task = fixture.make_task();
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{task, task});
    auto model = TaskOrdersModel(tasks);
    auto order = make_order(1);
    publish(task, order);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 1);
    tasks->remove(0);
    REQUIRE(model.get_size() == 1);
    publish(task, make_order(2));
    tasks->remove(0);
    REQUIRE(model.get_size() == 0);
    tasks->push(task);
    REQUIRE(model.get_size() == 2);
    QApplication::processEvents();
    REQUIRE(model.get_size() == 2);
    tasks->remove(0);
    REQUIRE(model.get_size() == 0);
  }

  UI_TEST_CASE("task_transactions") {
    auto fixture = Fixture();
    auto first = fixture.make_task();
    auto second = fixture.make_task();
    publish(first, make_order(1));
    publish(second, make_order(2));
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto model = TaskOrdersModel(tasks);
    auto operations = std::deque<OrdersModel::Operation>();
    model.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    tasks->transact([&] {
      tasks->push(first);
      tasks->push(second);
    });
    require_list_transaction<OrdersModel::Type>(operations, {
      OrdersModel::AddOperation(0), OrdersModel::AddOperation(1)});
  }

  UI_TEST_CASE("destruction_with_queued_orders") {
    auto fixture = Fixture();
    auto task = fixture.make_task();
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>(
      std::vector{task});
    auto operations = std::deque<OrdersModel::Operation>();
    {
      auto model = TaskOrdersModel(tasks);
      model.connect_operation_signal([&] (const auto& operation) {
        operations.push_back(operation);
      });
      publish(task, make_order(1));
    }
    publish(task, make_order(2));
    QApplication::processEvents();
    REQUIRE(operations.empty());
  }
}
