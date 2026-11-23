#include <QEventLoop>
#include <QLayout>
#include <QtTest/qtestkeyboard.h>
#include <QtTest/qtestmouse.h>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/StandardNodes/FirstNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

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
  UI_TEST_CASE("pin_alignment") {
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto fixture = Fixture();
    auto entry = BlotterTaskEntry();
    entry.m_task =
      std::make_shared<Task>(IntegerNode(1), DirectoryEntry(), Ref(profile));
    fixture.m_tasks->push(entry);
    fixture.m_view->resize(640, 240);
    fixture.m_view->show();
    QApplication::processEvents();
    auto find_pin = [] (QWidget& parent) -> Icon& {
      for(auto child : parent.findChildren<QWidget*>()) {
        auto icon = dynamic_cast<Icon*>(child);
        if(icon && icon->sizeHint() == scale(10, 10)) {
          return *icon;
        }
      }
      throw std::runtime_error("Pin icon not found.");
    };
    auto& table = fixture.table();
    auto& header = *table.get_header().get_item(0);
    INFO("Pinned column width: ", header.width());
    for(auto order : {TableHeaderItem::Order::NONE,
        TableHeaderItem::Order::ASCENDING,
        TableHeaderItem::Order::DESCENDING}) {
      auto model = header.get_model()->get();
      model.m_order = order;
      header.get_model()->set(model);
      QApplication::processEvents();
      auto row = table.get_body().find_item(TableIndex(0, 0));
      REQUIRE(row);
      auto& header_pin = find_pin(header);
      auto& row_pin = find_pin(*row);
      auto row_image = QRect(QPoint(
        (row_pin.width() - row_pin.sizeHint().width()) / 2,
        (row_pin.height() - row_pin.sizeHint().height()) / 2),
        row_pin.sizeHint());
      REQUIRE(header_pin.mapToGlobal(QPoint()).x() ==
        row_pin.mapToGlobal(row_image.topLeft()).x());
      REQUIRE(header_pin.visibleRegion().boundingRect() == header_pin.rect());
      REQUIRE(row_pin.visibleRegion().contains(row_image));
    }
  }

  UI_TEST_CASE("row_multi_selection") {
    auto environment = TestEnvironment();
    auto profile = UserProfile("", false, false, {}, {},
      get_default_additional_tag_database(), {}, {}, {},
      Clients(std::in_place_type<TestClients>, Ref(environment)));
    auto fixture = Fixture();
    for(auto i = 0; i != 4; ++i) {
      auto entry = BlotterTaskEntry();
      entry.m_task =
        std::make_shared<Task>(IntegerNode(i), DirectoryEntry(), Ref(profile));
      fixture.m_tasks->push(entry);
    }
    fixture.m_view->resize(640, 240);
    fixture.m_view->show();
    fixture.m_view->activateWindow();
    QApplication::processEvents();
    auto window = fixture.m_view->windowHandle();
    auto& table = fixture.table();
    auto& rows = std::as_const(*table.get_selection()->get_row_selection());
    auto click = [&] (TableIndex index, Qt::KeyboardModifiers modifiers) {
      auto item = table.get_body().find_item(index);
      REQUIRE(item);
      auto point = item->mapToGlobal(item->rect().center());
      QCursor::setPos(point);
      QTest::mouseClick(window, Qt::LeftButton, modifiers,
        fixture.m_view->mapFromGlobal(point));
      QApplication::processEvents();
    };
    auto is_highlighted = [&] (int row) {
      auto item = table.get_body().find_item(TableIndex(row, 1));
      REQUIRE(item);
      return find<BackgroundColor>(get_computed_block(*item->parentWidget())) ==
        BackgroundColor(QColor(0xE2E0FF));
    };
    click(TableIndex(0, 1), Qt::NoModifier);
    SUBCASE("toggle") {
      QTest::keyPress(window, Qt::Key_Control);
      click(TableIndex(2, 1), Qt::ControlModifier);
      REQUIRE(std::vector(rows.begin(), rows.end()) == std::vector{0, 2});
      REQUIRE(is_highlighted(0));
      REQUIRE(is_highlighted(2));
      click(TableIndex(0, 1), Qt::ControlModifier);
      QTest::keyRelease(window, Qt::Key_Control);
      REQUIRE(std::vector(rows.begin(), rows.end()) == std::vector{2});
      REQUIRE_FALSE(is_highlighted(0));
      REQUIRE(is_highlighted(2));
    }
    SUBCASE("range") {
      QTest::keyPress(window, Qt::Key_Shift);
      click(TableIndex(3, 1), Qt::ShiftModifier);
      QTest::keyRelease(window, Qt::Key_Shift);
      REQUIRE(std::vector(rows.begin(), rows.end()) ==
        std::vector{0, 1, 2, 3});
      for(auto i : rows) {
        REQUIRE(is_highlighted(i));
      }
    }
    SUBCASE("pin_button") {
      QTest::keyPress(window, Qt::Key_Control);
      click(TableIndex(1, 0), Qt::ControlModifier);
      QTest::keyRelease(window, Qt::Key_Control);
      REQUIRE(fixture.m_tasks->get(1).m_is_pinned);
      REQUIRE(std::vector(rows.begin(), rows.end()) == std::vector{0, 1});
    }
  }

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
    auto quantity = std::unique_ptr<CanvasNode>();
    auto is_literal = false;
    SUBCASE("literal") {
      quantity = std::make_unique<IntegerNode>(7);
      is_literal = true;
    }
    SUBCASE("computed") {
      auto builder = CanvasNodeBuilder(FirstNode());
      builder.Replace("expression", std::make_unique<IntegerNode>(7));
      quantity = builder.Make();
    }
    auto task = node.Replace(node.GetChildren().front(), std::move(quantity));
    auto entry = BlotterTaskEntry();
    entry.m_task =
      std::make_shared<Task>(*task, DirectoryEntry(), Ref(profile));
    fixture.m_tasks->push(entry);
    auto table = fixture.table().get_table();
    auto column = fixture.headers().indexOf("Quantity");
    REQUIRE(column >= 0);
    if(is_literal) {
      REQUIRE(table->get<Quantity>(0, column) == 7);
    } else {
      REQUIRE_FALSE(to_any(table->at(0, column)).has_value());
    }
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
    loop.exec();
    REQUIRE(std::any_cast<Quantity>(value) == 7);
    REQUIRE(table->get<Quantity>(0, column) == 7);
  }
}
