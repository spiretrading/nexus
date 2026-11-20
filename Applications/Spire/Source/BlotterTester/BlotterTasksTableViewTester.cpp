#include <QEventLoop>
#include <QLayout>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    std::shared_ptr<TaskMonitorsModel> m_monitors;
    std::shared_ptr<MonitoredTasksModel> m_tasks;
    std::unique_ptr<BlotterTasksTableView> m_view;

    Fixture()
        : m_monitors(make_default_task_monitors()),
          m_tasks(std::make_shared<MonitoredTasksModel>(m_monitors)) {
      initialize_resources();
      auto selection = std::make_shared<TableSelectionModel>(
        std::make_shared<TableEmptySelectionModel>(),
        std::make_shared<ListMultiSelectionModel>(),
        std::make_shared<ListEmptySelectionModel>());
      m_view = std::make_unique<BlotterTasksTableView>(
        m_tasks, m_monitors, std::move(selection));
    }

    TableView& table() {
      return dynamic_cast<TableView&>(*m_view->layout()->itemAt(0)->widget());
    }

    auto headers() {
      auto names = QStringList();
      for(auto& item : std::as_const(*table().get_header().get_items())) {
        names.push_back(item.m_name);
      }
      REQUIRE(table().get_table()->get_column_size() == names.size());
      return names;
    }
  };
}

TEST_SUITE("BlotterTasksTableView") {
  UI_TEST_CASE("monitor_headers") {
    auto fixture = Fixture();
    auto names = QStringList{"Pinned", "Name", "ID", "State", "Ticker",
      "Side", "Price", "Quantity", "Volume"};
    REQUIRE(fixture.headers() == names);
    auto monitor = fixture.m_monitors->get(2);
    monitor.m_name = "Limit";
    fixture.m_monitors->set(2, monitor);
    names[6] = "Limit";
    REQUIRE(fixture.headers() == names);
    fixture.m_monitors->remove(0);
    names.removeAt(4);
    REQUIRE(fixture.headers() == names);
    fixture.m_monitors->move(0, 2);
    names.move(4, 6);
    REQUIRE(fixture.headers() == names);
    fixture.m_monitors->insert(BlotterTaskMonitor("Custom", nullptr), 1);
    names.insert(5, "Custom");
    REQUIRE(fixture.headers() == names);
  }

  UI_TEST_CASE("monitor_value") {
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto fixture = Fixture();
    auto node = CustomNode(
      "Task", {CustomNode::Child("quantity", IntegerType::GetInstance())});
    auto task = node.Replace(
      node.GetChildren().front(), std::make_unique<IntegerNode>(7));
    auto entry = BlotterTaskEntry();
    entry.m_task =
      std::make_shared<Task>(*task, DirectoryEntry(), Ref(profile));
    fixture.m_tasks->push(entry);
    auto table = fixture.table().get_table();
    auto column = fixture.headers().indexOf("Quantity");
    REQUIRE(column >= 0);
    REQUIRE_FALSE(to_any(table->at(0, column)).has_value());
    auto loop = QEventLoop();
    auto value = std::any();
    auto connection = boost::signals2::scoped_connection(
      table->connect_operation_signal([&] (const auto& operation) {
        if(auto update = std::get_if<TableModel::UpdateOperation>(&operation)) {
          if(update->m_row == 0 && update->m_column == column) {
            value = update->m_value;
            loop.quit();
          }
        }
      }));
    entry.m_task->Execute();
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    REQUIRE(std::any_cast<Quantity>(value) == 7);
    REQUIRE(table->get<Quantity>(0, column) == 7);
  }
}
