#include <sstream>
#include <boost/optional/optional_io.hpp>
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/OrderExecutionReportsModel.hpp"
#include "Spire/Spire/ExportTable.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/OrderStatusFilterPanel.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  struct Fixture {
    std::shared_ptr<ArrayListModel<ExecutionReportEntry>> m_reports;
    std::unique_ptr<BlotterExecutionsView> m_view;
    TableView* m_table;

    Fixture()
        : m_reports(std::make_shared<ArrayListModel<ExecutionReportEntry>>()) {
      m_view = std::make_unique<BlotterExecutionsView>(m_reports);
      m_table = static_cast<TableView*>(m_view->focusProxy());
    }
  };

  auto make_order(OrderId id, OrderType type) {
    auto fields = OrderFields();
    fields.m_type = type;
    fields.m_ticker = parse_ticker("RY.TSX");
    fields.m_side = Side::BID;
    fields.m_price = Money(12);
    fields.m_quantity = 100;
    return std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, id, time_from_string("2026-11-20 10:00:00")));
  }
}

TEST_SUITE("BlotterExecutionsView") {
  UI_TEST_CASE("report_columns") {
    auto fixture = Fixture();
    auto& table = *fixture.m_table->get_table();
    REQUIRE(table.get_column_size() == 15);
    REQUIRE(table.get_row_size() == 0);
    auto order = make_order(12, OrderType::LIMIT);
    auto report = ExecutionReport(12, time_from_string("2026-11-20 10:01:00"));
    report.m_sequence = 2;
    report.m_status = OrderStatus::PARTIALLY_FILLED;
    report.m_last_quantity = 40;
    report.m_last_price = Money(11);
    report.m_last_market = "XTSE";
    report.m_liquidity_flag = "A";
    report.m_execution_fee = Money(1);
    report.m_processing_fee = Money(2);
    report.m_commission = Money(3);
    report.m_text = "Partial fill, \"passive\"";
    fixture.m_reports->push(ExecutionReportEntry(order, report));
    REQUIRE(table.get<ptime>(0, 0) == report.m_timestamp);
    REQUIRE(table.get<OrderId>(0, 1) == 12);
    REQUIRE(table.get<Side>(0, 2) == Side::BID);
    REQUIRE(table.get<Ticker>(0, 3) == order->get_info().m_fields.m_ticker);
    REQUIRE(table.get<OrderStatus>(0, 4) == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(table.get<Quantity>(0, 5) == 100);
    REQUIRE(table.get<optional<Quantity>>(0, 6) == Quantity(40));
    REQUIRE(table.get<optional<Money>>(0, 7) == Money(12));
    REQUIRE(table.get<optional<Money>>(0, 8) == Money(11));
    REQUIRE(table.get<std::string>(0, 9) == "XTSE");
    REQUIRE(table.get<std::string>(0, 10) == "A");
    REQUIRE(table.get<Money>(0, 11) == Money(1));
    REQUIRE(table.get<Money>(0, 12) == Money(2));
    REQUIRE(table.get<Money>(0, 13) == Money(3));
    REQUIRE(table.get<std::string>(0, 14) == "Partial fill, \"passive\"");
    REQUIRE(to_text(table.at(0, 6)) == to_text(Quantity(40)));
    auto output = std::ostringstream();
    export_as_csv(table, {}, output);
    REQUIRE(output.str().find(
      ",100,\"40\",\"12.00\",\"11.00\",") != std::string::npos);
    REQUIRE(output.str().ends_with("\"Partial fill, \"\"passive\"\"\""));
  }

  UI_TEST_CASE("unfilled_market_order") {
    auto fixture = Fixture();
    auto order = make_order(1, OrderType::MARKET);
    auto report = order->get_publisher().get_snapshot()->front();
    fixture.m_reports->push(ExecutionReportEntry(order, report));
    auto& table = *fixture.m_table->get_table();
    REQUIRE(!table.get<optional<Quantity>>(0, 6));
    REQUIRE(!table.get<optional<Money>>(0, 7));
    REQUIRE(!table.get<optional<Money>>(0, 8));
    REQUIRE(table.get<std::string>(0, 9).empty());
    REQUIRE(to_text(table.at(0, 6)).isEmpty());
    REQUIRE(to_text(table.at(0, 7)).isEmpty());
    REQUIRE(to_text(table.at(0, 8)).isEmpty());
  }

  UI_TEST_CASE("report_sorting") {
    auto fixture = Fixture();
    auto order = make_order(1, OrderType::LIMIT);
    auto initial = order->get_publisher().get_snapshot()->front();
    auto first = make_update(initial, OrderStatus::PARTIALLY_FILLED,
      time_from_string("2026-11-20 10:01:00"));
    first.m_last_quantity = 20;
    auto second = make_update(first, OrderStatus::PARTIALLY_FILLED,
      time_from_string("2026-11-20 10:02:00"));
    second.m_last_quantity = 3;
    fixture.m_reports->push(ExecutionReportEntry(order, initial));
    fixture.m_reports->push(ExecutionReportEntry(order, first));
    fixture.m_reports->push(ExecutionReportEntry(order, second));
    auto& displayed = *fixture.m_table->get_body().get_table();
    REQUIRE(displayed.get<ptime>(0, 0) == second.m_timestamp);
    REQUIRE(displayed.get<ptime>(2, 0) == initial.m_timestamp);
    auto sorted = SortedTableModel(fixture.m_table->get_table());
    sorted.set_column_order({{6, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(!sorted.get<optional<Quantity>>(0, 6));
    REQUIRE(sorted.get<optional<Quantity>>(1, 6) == Quantity(3));
    REQUIRE(sorted.get<optional<Quantity>>(2, 6) == Quantity(20));
    fixture.m_reports->remove(2);
    REQUIRE(displayed.get_row_size() == 2);
    REQUIRE(displayed.get<ptime>(0, 0) == first.m_timestamp);
  }

  UI_TEST_CASE("time_sort_sequence") {
    auto reports = std::make_shared<ArrayListModel<ExecutionReportEntry>>();
    auto order = make_order(1, OrderType::LIMIT);
    auto timestamp = time_from_string("2026-11-20 10:01:00");
    auto append = [&] (int sequence, ptime time, OrderStatus status) {
      auto report = ExecutionReport(1, time);
      report.m_sequence = sequence;
      report.m_status = status;
      report.m_last_quantity = sequence;
      reports->push(ExecutionReportEntry(order, report));
    };
    append(8, timestamp - seconds(1), OrderStatus::NEW);
    append(3, timestamp, OrderStatus::PARTIALLY_FILLED);
    append(1, timestamp, OrderStatus::PARTIALLY_FILLED);
    append(1, timestamp + seconds(1), OrderStatus::NEW);
    append(2, timestamp, OrderStatus::PARTIALLY_FILLED);
    auto view = BlotterExecutionsView(reports);
    auto& table = *static_cast<TableView*>(view.focusProxy());
    auto& displayed = *table.get_body().get_table();
    auto quantities = [&] {
      auto values = std::vector<Quantity>();
      for(auto i = 0; i != displayed.get_row_size(); ++i) {
        values.push_back(*displayed.get<optional<Quantity>>(i, 6));
      }
      return values;
    };
    REQUIRE(quantities() == std::vector<Quantity>{1, 3, 2, 1, 8});
    auto& sorted = static_cast<SortedTableModel&>(displayed);
    sorted.set_column_order({{0, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(quantities() == std::vector<Quantity>{8, 1, 2, 3, 1});
    view.show();
    QApplication::processEvents();
    auto& header = table.get_header();
    header.get_item(4)->is_filter_open()->set(true);
    auto panel = static_cast<OrderStatusFilterPanel*>(nullptr);
    for(auto child : header.findChildren<QWidget*>()) {
      if(auto filter = dynamic_cast<OrderStatusFilterPanel*>(child)) {
        panel = filter;
        break;
      }
    }
    REQUIRE(panel);
    panel->get_current()->push(OrderStatus::PARTIALLY_FILLED);
    REQUIRE(quantities() == std::vector<Quantity>{1, 2, 3});
    append(4, timestamp, OrderStatus::PARTIALLY_FILLED);
    REQUIRE(quantities() == std::vector<Quantity>{1, 2, 3, 4});
    reports->remove(4);
    REQUIRE(quantities() == std::vector<Quantity>{1, 3, 4});
    sorted.set_column_order({{0, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(quantities() == std::vector<Quantity>{4, 3, 1});
    clear(*panel->get_current());
    REQUIRE(quantities() == std::vector<Quantity>{1, 4, 3, 1, 8});
  }

  UI_TEST_CASE("status_filter") {
    auto fixture = Fixture();
    auto order = make_order(1, OrderType::LIMIT);
    auto initial = order->get_publisher().get_snapshot()->front();
    auto accepted = make_update(initial, OrderStatus::NEW,
      time_from_string("2026-11-20 10:01:00"));
    fixture.m_reports->push(ExecutionReportEntry(order, initial));
    fixture.m_reports->push(ExecutionReportEntry(order, accepted));
    fixture.m_view->show();
    QApplication::processEvents();
    auto& header = fixture.m_table->get_header();
    header.get_item(4)->is_filter_open()->set(true);
    auto panel = static_cast<OrderStatusFilterPanel*>(nullptr);
    for(auto child : header.findChildren<QWidget*>()) {
      if(auto filter = dynamic_cast<OrderStatusFilterPanel*>(child)) {
        panel = filter;
        break;
      }
    }
    REQUIRE(panel);
    panel->get_current()->push(OrderStatus::NEW);
    auto& displayed = *fixture.m_table->get_body().get_table();
    REQUIRE(displayed.get_row_size() == 1);
    REQUIRE(displayed.get<OrderStatus>(0, 4) == OrderStatus::NEW);
    REQUIRE(
      header.get_items()->get(4).m_filter == TableFilter::Filter::FILTERED);
    fixture.m_reports->push(ExecutionReportEntry(order, make_update(
      accepted, OrderStatus::FILLED, time_from_string("2026-11-20 10:02:00"))));
    REQUIRE(displayed.get_row_size() == 1);
    clear(*panel->get_current());
    REQUIRE(displayed.get_row_size() == 3);
    REQUIRE(
      header.get_items()->get(4).m_filter == TableFilter::Filter::UNFILTERED);
  }

  UI_TEST_CASE("live_reports") {
    auto order = make_order(1, OrderType::LIMIT);
    auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>(
      std::vector<OrdersModel::Type>{order});
    auto reports = std::make_shared<OrderExecutionReportsModel>(orders);
    auto view = BlotterExecutionsView(reports);
    auto table = static_cast<TableView*>(view.focusProxy())->get_table();
    REQUIRE(table->get_row_size() == 1);
    Nexus::Tests::accept(*order);
    Nexus::Tests::fill(*order, Money(11), 40);
    QApplication::processEvents();
    REQUIRE(table->get_row_size() == 3);
    REQUIRE(table->get<OrderStatus>(0, 4) == OrderStatus::PENDING_NEW);
    REQUIRE(table->get<OrderStatus>(1, 4) == OrderStatus::NEW);
    REQUIRE(table->get<optional<Quantity>>(2, 6) == Quantity(40));
    orders->remove(0);
    REQUIRE(table->get_row_size() == 0);
  }
}
