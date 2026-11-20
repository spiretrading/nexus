#include <algorithm>
#include <array>
#include <Beam/Queues/Queue.hpp>
#include <QtTest/qtestmouse.h>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("BlotterTasksView") {
  UI_TEST_CASE("cancel_selected_tasks") {
    initialize_resources();
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
    auto states = std::make_shared<Queue<Task::StateEntry>>();
    for(auto i = 0; i != 3; ++i) {
      auto entry = BlotterTaskEntry();
      entry.m_task =
        std::make_shared<Task>(IntegerNode(i), DirectoryEntry(), Ref(profile));
      entry.m_task->GetPublisher().monitor(states);
      tasks->insert(entry, 0);
    }
    auto view = BlotterTasksView(std::make_shared<CompositeBlotterModel>(
      std::make_shared<LocalTextModel>("Test"),
      std::make_shared<LocalBooleanModel>(false),
      std::make_shared<LocalBooleanModel>(false), tasks,
      std::make_shared<ArrayListModel<BlotterTaskMonitor>>()));
    view.resize(640, 240);
    view.show();
    QApplication::processEvents();
    auto table = static_cast<TableView*>(nullptr);
    auto cancel = static_cast<QWidget*>(nullptr);
    for(auto widget : view.findChildren<QWidget*>()) {
      if(auto candidate = dynamic_cast<TableView*>(widget)) {
        table = candidate;
      } else if(widget->accessibleName() == "Cancel tasks") {
        cancel = widget;
      }
    }
    REQUIRE(table);
    REQUIRE(cancel);
    auto requests = std::vector<std::vector<std::shared_ptr<Task>>>();
    view.connect_cancel_tasks_signal(
      [&] (const auto& selected) { requests.push_back(selected); });
    auto click = [&] (QWidget& widget) {
      auto point = widget.mapToGlobal(widget.rect().center());
      QCursor::setPos(point);
      QTest::mouseClick(view.windowHandle(), Qt::LeftButton, Qt::NoModifier,
        view.mapFromGlobal(point));
      QApplication::processEvents();
    };
    click(*cancel);
    REQUIRE(requests.empty());
    const auto ID_COLUMN = 2;
    click(*table->get_header().get_item(ID_COLUMN));
    auto& body = table->get_body();
    REQUIRE(body.get_table()->get<int>(0, ID_COLUMN) ==
      tasks->get(2).m_task->GetId());
    auto rows = body.get_selection()->get_row_selection();
    rows->push(0);
    rows->push(1);
    click(*cancel);
    REQUIRE(requests.size() == 1);
    REQUIRE(std::ranges::is_permutation(requests.front(),
      std::array{tasks->get(2).m_task, tasks->get(1).m_task}));
    REQUIRE(!states->try_pop());
  }
}
