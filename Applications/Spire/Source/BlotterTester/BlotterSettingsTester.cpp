#include <array>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/optional/optional_io.hpp>
#include <QHeaderView>
#include <QMetaObject>
#include <QSplitter>
#include <QStandardItemModel>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterLinkMenu.hpp"
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/LegacyBlotterSettings.hpp"
#include "Spire/Blotter/LegacyBlotterWindowSettings.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/OrderStatusFilterPanel.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Spire;

namespace {
  struct LegacyWindow {
    std::string m_name;
    QByteArray m_geometry;
    QByteArray m_splitter;
    LegacyBlotterWindowSettings::TableState m_positions;
    LegacyBlotterWindowSettings::TableState m_orders;
    LegacyBlotterWindowSettings::TableState m_executions;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version);
  };

  template<IsShuttle S>
  void LegacyWindow::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("blotter_name", m_name);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("splitter_state", m_splitter);
    shuttle.shuttle("open_positions_widget_state", m_positions);
    shuttle.shuttle("order_log_widget_state", m_orders);
    shuttle.shuttle("activity_log_widget_state", m_executions);
  }

  template<typename T>
  auto read_legacy(const auto& value) {
    auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    auto buffer = SharedBuffer();
    auto sender = BinarySender<SharedBuffer>(Ref(registry));
    sender.set(Ref(buffer));
    sender.shuttle(value);
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
    receiver.set(Ref(buffer));
    auto restored = T();
    receiver.shuttle(restored);
    return restored;
  }

  auto make_legacy_table(int count) {
    auto model = QStandardItemModel(0, count);
    auto header = QHeaderView(Qt::Horizontal);
    header.setModel(&model);
    header.setSectionsMovable(true);
    for(auto i = 0; i != count; ++i) {
      header.resizeSection(i, scale_width(90 + 7 * i));
    }
    header.moveSection(count - 1, 0);
    header.hideSection(1);
    header.setSortIndicator(count - 1, Qt::DescendingOrder);
    header.setSortIndicatorShown(true);
    return LegacyBlotterWindowSettings::TableState(
      header.saveGeometry(), header.saveState());
  }

  void require_legacy_table(TableView& table, const auto& columns) {
    auto& header = table.get_header();
    auto count = static_cast<int>(columns.size());
    for(auto visual = 0; visual != count; ++visual) {
      auto index = visual - 1;
      if(visual == 0) {
        index = count - 1;
      }
      REQUIRE(header.get_column_order()->get(visual) == columns[index]);
      REQUIRE(header.get_widths()->get(visual) == scale_width(90 + 7 * index));
      REQUIRE(header.get_item(visual)->isHidden() == (index == 1));
    }
    auto& sort = static_cast<const SortedTableModel&>(
      *table.get_body().get_table()).get_column_order();
    REQUIRE(sort.size() == 1);
    REQUIRE(sort.front().m_index == columns.back());
    REQUIRE(sort.front().m_order == SortedTableModel::Ordering::DESCENDING);
  }

  struct Fixture {
    TestEnvironment m_environment;
    UserProfile m_profile;

    Fixture()
        : m_profile("", false, false, {}, {},
            get_default_additional_tag_database(), {}, {}, {},
            Clients(std::in_place_type<TestClients>, Ref(m_environment))) {
      initialize_resources();
    }

    ~Fixture() {
      flush_pending_routines();
    }

    auto& create(const QString& name) {
      auto account = m_profile.GetClients().get_service_locator_client().
        get_account();
      return m_profile.GetBlotterRegistry().create(name, account);
    }
  };

  template<typename T>
  T& find_widget(QWidget& parent) {
    for(auto child : parent.findChildren<QWidget*>()) {
      if(auto widget = dynamic_cast<T*>(child)) {
        return *widget;
      }
    }
    throw std::runtime_error("Widget not found.");
  }

  auto round_trip(const auto& value) {
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(out(registry));
    auto buffer = SharedBuffer();
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    sender.set(Ref(buffer));
    sender.shuttle(value);
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.set(Ref(buffer));
    auto restored = std::remove_cvref_t<decltype(value)>();
    receiver.shuttle(restored);
    return restored;
  }

  void configure_table(TableView& table) {
    auto& header = table.get_header();
    auto last = header.get_items()->get_size() - 1;
    header.get_items()->move(last, 1);
    header.get_widths()->set(1, scale_width(173));
    table.hide_column(2);
    auto item = header.get_items()->get(1);
    item.m_order = TableHeaderItem::Order::DESCENDING;
    header.get_items()->set(1, item);
    static_cast<SortedTableModel&>(*table.get_body().get_table()).
      set_column_order({{last, SortedTableModel::Ordering::DESCENDING},
        {0, SortedTableModel::Ordering::ASCENDING}});
  }

  void require_table(TableView& table) {
    auto& header = table.get_header();
    REQUIRE(header.get_column_order()->get(1) ==
      header.get_items()->get_size() - 1);
    REQUIRE(header.get_widths()->get(1) == scale_width(173));
    REQUIRE(header.get_item(2)->isHidden());
    REQUIRE(
      header.get_items()->get(1).m_order == TableHeaderItem::Order::DESCENDING);
    auto& sort = static_cast<SortedTableModel&>(
      *table.get_body().get_table()).get_column_order();
    REQUIRE(sort.size() == 2);
    REQUIRE(sort[0].m_index == header.get_items()->get_size() - 1);
    REQUIRE(sort[0].m_order == SortedTableModel::Ordering::DESCENDING);
    REQUIRE(sort[1].m_index == 0);
    REQUIRE(sort[1].m_order == SortedTableModel::Ordering::ASCENDING);
  }
}

TEST_SUITE("BlotterSettings") {
  UI_TEST_CASE("legacy_configuration") {
    auto fixture = Fixture();
    auto account = fixture.m_profile.GetClients().get_service_locator_client().
      get_account();
    auto legacy = LegacyBlotterSettings();
    for(auto name : {"Global", "First", "Second", "First"}) {
      auto properties = LegacyBlotterSettings::Properties();
      properties.m_name = name;
      properties.m_account = account;
      properties.m_is_consolidated = properties.m_name == "Global";
      properties.m_order_log_properties.m_order_status_filter_type =
        LegacyBlotterSettings::OrderLogProperties::OrderStatusFilterType::
          ALL_ORDERS;
      legacy.m_blotters.push_back(std::move(properties));
    }
    legacy.m_blotters[1].m_links = {"Second", "Global", "Missing"};
    auto node = BlotterTaskMonitorNode();
    legacy.m_blotters[1].m_task_properties.m_task_monitors.push_back(
      LegacyBlotterSettings::TaskMonitor("Custom", node.Replace(
        node.GetChildren().front(), std::make_unique<IntegerNode>(42))));
    legacy.m_active_blotter = "First";
    auto converted = round_trip(BlotterRegistry::State(
      read_legacy<LegacyBlotterSettings>(legacy)));
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    registry.restore(converted);
    REQUIRE(registry.get_blotters()->get_size() == 3);
    REQUIRE(registry.get_pinned()->get_size() == 2);
    auto first = registry.get_active()->get();
    REQUIRE(first->get_name()->get() == "First");
    REQUIRE(first->get_account() == account);
    REQUIRE(first->get_task_monitors()->get_size() == 1);
    auto& monitor = first->get_task_monitors()->get(0);
    REQUIRE(monitor.m_name == "Custom");
    REQUIRE(dynamic_cast<const IntegerNode&>(
      monitor.m_expression->GetChildren().front()).GetValue() == 42);
    REQUIRE(!monitor.m_default_width);
    auto second = registry.get_blotters()->get(2);
    REQUIRE(second->get_task_monitors()->get_size() == 0);
    registry.get(*second).add(IntegerNode(8));
    REQUIRE(first->get_tasks()->get_size() == 1);
    legacy.m_active_blotter = "Missing";
    auto fallback = Fixture();
    fallback.m_profile.GetBlotterRegistry().restore(
      BlotterRegistry::State(legacy));
    REQUIRE(fallback.m_profile.GetBlotterRegistry().get_active()->get()->
      is_consolidated());
  }

  UI_TEST_CASE("legacy_window_layout") {
    auto fixture = Fixture();
    auto& controller = fixture.create("Legacy");
    controller.get_model()->is_pinned()->set(true);
    auto legacy = LegacyWindow();
    legacy.m_name = "Legacy";
    auto old_window = QWidget();
    old_window.resize(900, 700);
    old_window.move(100, 100);
    legacy.m_geometry = old_window.saveGeometry();
    auto splitter = QSplitter(Qt::Vertical);
    splitter.addWidget(new QWidget());
    splitter.addWidget(new QWidget());
    splitter.resize(800, 800);
    splitter.setSizes({600, 200});
    legacy.m_splitter = splitter.saveState();
    legacy.m_positions = make_legacy_table(7);
    legacy.m_orders = make_legacy_table(11);
    legacy.m_executions = make_legacy_table(13);
    auto settings = std::unique_ptr<LegacyUI::WindowSettings>(
      std::make_unique<LegacyBlotterWindowSettings>(
        read_legacy<LegacyBlotterWindowSettings>(legacy)));
    auto restored = round_trip(settings);
    auto window = restored->Reopen(Ref(fixture.m_profile));
    REQUIRE(window);
    REQUIRE(controller.is_open()->get());
    REQUIRE(window->width() == 900);
    auto frame_height = GetSystemMetrics(SM_CYCAPTION) +
      GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
    REQUIRE(window->height() == 700 + frame_height);
    REQUIRE(find_widget<SplitView>(*window).get_primary_ratio() == 0.75);
    auto& tabs = find_widget<TabView>(*window);
    REQUIRE(tabs.get_current()->get() == 0);
    require_legacy_table(
      find_widget<TableView>(find_widget<BlotterPositionsView>(*window)),
      std::array{0, 1, 2, 3, 4, 5, 6});
    tabs.get_current()->set(2);
    require_legacy_table(
      find_widget<TableView>(find_widget<OrderLogView>(*window)),
      std::array{0, 1, 2, 3, 9, 7, 4, 8, 6, 5, 10});
    tabs.get_current()->set(3);
    auto& executions =
      find_widget<TableView>(find_widget<BlotterExecutionsView>(*window));
    require_legacy_table(executions,
      std::array{0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14});
    REQUIRE(executions.get_header().get_column_order()->get(13) == 5);
    REQUIRE(executions.get_header().get_column_order()->get(14) == 7);
    auto modern = round_trip(
      dynamic_cast<BlotterWindow&>(*window).GetWindowSettings());
    controller.close();
    window = modern->Reopen(Ref(fixture.m_profile));
    REQUIRE(window);
    REQUIRE(find_widget<TabView>(*window).get_current()->get() == 3);
    REQUIRE(find_widget<SplitView>(*window).get_primary_ratio() == 0.75);
    require_legacy_table(
      find_widget<TableView>(find_widget<BlotterExecutionsView>(*window)),
      std::array{0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14});
    legacy.m_geometry.clear();
    legacy.m_splitter = QByteArray("invalid");
    legacy.m_positions.m_table_state = QByteArray("invalid");
    legacy.m_orders.m_table_state = QByteArray("invalid");
    legacy.m_executions.m_table_state = QByteArray("invalid");
    auto invalid = read_legacy<LegacyBlotterWindowSettings>(legacy);
    invalid.Apply(Ref(fixture.m_profile), out(*window));
    REQUIRE(find_widget<SplitView>(*window).get_primary_ratio() == 0.75);
    require_legacy_table(
      find_widget<TableView>(find_widget<BlotterExecutionsView>(*window)),
      std::array{0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14});
    legacy.m_name = "Missing";
    REQUIRE(!read_legacy<LegacyBlotterWindowSettings>(legacy).Reopen(
      Ref(fixture.m_profile)));
  }

  UI_TEST_CASE("single_blotter") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto& first = fixture.create("First");
    auto& second = fixture.create("Second");
    first.get_model()->is_pinned()->set(true);
    auto& monitors = *first.get_model()->get_task_monitors();
    auto monitor = BlotterTaskMonitorNode();
    monitors.push(BlotterTaskMonitor("Custom", monitor.Replace(
      monitor.GetChildren().front(), std::make_unique<IntegerNode>(59)), 177));
    auto count = monitors.get_size();
    auto& window = first.open();
    find_widget<TabView>(window).get_current()->set(3);
    find_widget<SplitView>(window).set_primary_ratio(0.6);
    first.close();
    auto settings = round_trip(BlotterSettings(first));
    first.get_model()->get_name()->set("Changed");
    monitors.remove(count - 1);
    registry.set_active(*second.get_model());
    settings.apply(first);
    REQUIRE(first.get_model()->get_name()->get() == "First");
    REQUIRE(monitors.get_size() == count);
    auto& restored = monitors.get(count - 1);
    REQUIRE(restored.m_name == "Custom");
    REQUIRE(restored.m_default_width == 177);
    REQUIRE(dynamic_cast<const IntegerNode&>(
      restored.m_expression->GetChildren().front()).GetValue() == 59);
    REQUIRE_FALSE(first.is_open()->get());
    auto& reopened = first.open();
    REQUIRE(find_widget<TabView>(reopened).get_current()->get() == 3);
    REQUIRE(find_widget<SplitView>(reopened).get_primary_ratio() == 0.6);
    REQUIRE(registry.get_blotters()->get_size() == 3);
    REQUIRE(registry.get_active()->get() == second.get_model());
    REQUIRE(second.get_model()->get_name()->get() == "Second");
    REQUIRE_FALSE(second.is_open()->get());
  }

  UI_TEST_CASE("configuration") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    auto& first = fixture.create("First");
    auto& second = fixture.create("Second");
    first.get_model()->is_pinned()->set(true);
    second.get_model()->is_pinned()->set(true);
    auto& window = first.open();
    auto& tasks = find_widget<BlotterTasksView>(window);
    REQUIRE(QMetaObject::invokeMethod(&tasks, "customContextMenuRequested",
      Qt::DirectConnection, Q_ARG(QPoint, QPoint())));
    auto& links = find_widget<BlotterLinkMenu>(window);
    REQUIRE(links.get_count() == 2);
    auto& menu = find_widget<ContextMenu>(window);
    dynamic_cast<OverlayPanel&>(*menu.window()).set_closed_on_focus_out(false);
    dynamic_cast<OverlayPanel&>(*links.window()).set_closed_on_focus_out(false);
    links.window()->show();
    QApplication::processEvents();
    find_widget<CheckBox>(*links.get_menu_item(0)).get_current()->set(true);
    second.add(IntegerNode(7));
    REQUIRE(first.get_model()->get_tasks()->get_size() == 1);
    links.window()->hide();
    first.get_model()->get_name()->set("Same name");
    second.get_model()->get_name()->set("Same name");
    auto monitor = BlotterTaskMonitorNode();
    auto expression = monitor.Replace(
      monitor.GetChildren().front(), std::make_unique<IntegerNode>(42));
    auto& monitors = *first.get_model()->get_task_monitors();
    monitors.push(BlotterTaskMonitor("Custom", std::move(expression), 177));
    monitors.push(BlotterTaskMonitor("Empty", nullptr, boost::none));
    configure_table(find_widget<TableView>(tasks));
    registry.set_active(*first.get_model());
    first.close();
    fixture.create("Temporary");
    auto settings = round_trip(registry.save_state());
    auto restored = Fixture();
    auto& restored_registry = restored.m_profile.GetBlotterRegistry();
    restored_registry.restore(settings);
    REQUIRE(restored_registry.get_blotters()->get_size() == 3);
    REQUIRE(restored_registry.get_pinned()->get_size() == 2);
    auto restored_first = restored_registry.get_blotters()->get(1);
    auto restored_second = restored_registry.get_blotters()->get(2);
    REQUIRE(restored_registry.get_active()->get() == restored_first);
    REQUIRE(restored_first->get_name()->get() == "Same name");
    REQUIRE(restored_second->get_name()->get() == "Same name");
    REQUIRE(restored_first->get_account() == first.get_model()->get_account());
    auto& restored_monitors = *restored_first->get_task_monitors();
    REQUIRE(restored_monitors.get_size() == monitors.get_size());
    auto& custom = restored_monitors.get(restored_monitors.get_size() - 2);
    REQUIRE(custom.m_name == "Custom");
    REQUIRE(custom.m_default_width == 177);
    REQUIRE(dynamic_cast<const IntegerNode&>(
      custom.m_expression->GetChildren().front()).GetValue() == 42);
    REQUIRE(
      !restored_monitors.get(restored_monitors.get_size() - 1).m_expression);
    auto& controller = restored_registry.get(*restored_first);
    REQUIRE_FALSE(controller.is_open()->get());
    auto& restored_window = controller.open();
    auto& restored_table = find_widget<TableView>(
      find_widget<BlotterTasksView>(restored_window));
    require_table(restored_table);
    auto renamed = restored_monitors.get(restored_monitors.get_size() - 1);
    renamed.m_name = "Renamed";
    restored_monitors.set(restored_monitors.get_size() - 1, renamed);
    REQUIRE(
      restored_table.get_header().get_items()->get(1).m_name == "Renamed");
    restored_registry.get(*restored_second).add(IntegerNode(8));
    REQUIRE(restored_first->get_tasks()->get_size() == 1);
  }

  UI_TEST_CASE("window_layout") {
    auto fixture = Fixture();
    auto& controller = fixture.create("Layout");
    controller.get_model()->is_pinned()->set(true);
    auto& window = controller.open();
    window.resize(900, 700);
    auto& tabs = find_widget<TabView>(window);
    auto& split = find_widget<SplitView>(window);
    split.set_primary_ratio(0.65);
    configure_table(
      find_widget<TableView>(find_widget<BlotterTasksView>(window)));
    for(auto tab : {0, 1, 2, 3}) {
      tabs.get_current()->set(tab);
      if(tab == 0) {
        configure_table(
          find_widget<TableView>(find_widget<BlotterPositionsView>(window)));
      } else if(tab == 1) {
        configure_table(
          find_widget<TableView>(find_widget<BlotterProfitLossView>(window)));
      } else if(tab == 2) {
        configure_table(
          find_widget<TableView>(find_widget<OrderLogView>(window)));
      } else {
        auto& table =
          find_widget<TableView>(find_widget<BlotterExecutionsView>(window));
        configure_table(table);
        auto panel = std::unique_ptr<QWidget>(
          table.get_header().get_filter()->make_filter_widget(4, table));
        dynamic_cast<OrderStatusFilterPanel&>(*panel).get_current()->push(
          OrderStatus::FILLED);
      }
    }
    QApplication::processEvents();
    auto size = window.size();
    auto settings = BlotterWindowSettings();
    test_round_trip_shuttle(BlotterWindowSettings(window), [&] (auto restored) {
      settings = round_trip(restored);
    });
    controller.close();
    auto restored =
      dynamic_cast<BlotterWindow*>(settings.Reopen(Ref(fixture.m_profile)));
    REQUIRE(restored);
    REQUIRE(controller.is_open()->get());
    REQUIRE(restored->size() == size);
    REQUIRE(find_widget<SplitView>(*restored).get_primary_ratio() == 0.65);
    auto& restored_tabs = find_widget<TabView>(*restored);
    REQUIRE(restored_tabs.get_current()->get() == 3);
    require_table(
      find_widget<TableView>(find_widget<BlotterTasksView>(*restored)));
    auto& executions =
      find_widget<TableView>(find_widget<BlotterExecutionsView>(*restored));
    require_table(executions);
    auto panel = std::unique_ptr<QWidget>(
      executions.get_header().get_filter()->make_filter_widget(4, executions));
    auto statuses = dynamic_cast<OrderStatusFilterPanel&>(*panel).get_current();
    REQUIRE(statuses->get_size() == 1);
    REQUIRE(statuses->get(0) == OrderStatus::FILLED);
    restored_tabs.get_current()->set(0);
    require_table(
      find_widget<TableView>(find_widget<BlotterPositionsView>(*restored)));
    restored_tabs.get_current()->set(2);
    require_table(find_widget<TableView>(find_widget<OrderLogView>(*restored)));
    restored_tabs.get_current()->set(1);
    require_table(
      find_widget<TableView>(find_widget<BlotterProfitLossView>(*restored)));
    REQUIRE(settings.Reopen(Ref(fixture.m_profile)) == restored);
  }

  UI_TEST_CASE("temporary_active_blotter") {
    auto fixture = Fixture();
    auto& registry = fixture.m_profile.GetBlotterRegistry();
    registry.set_active(*fixture.create("Temporary").get_model());
    auto settings = round_trip(registry.save_state());
    auto restored = Fixture();
    auto& restored_registry = restored.m_profile.GetBlotterRegistry();
    restored_registry.restore(settings);
    REQUIRE(restored_registry.get_blotters()->get_size() == 1);
    REQUIRE(restored_registry.get_active()->get()->is_consolidated());
  }
}
