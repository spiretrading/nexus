#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/SaleConditonBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({"Time", "Time",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({"Price", "Px",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({"Size", "Sz",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({"Market", "Mkt",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({"Condition", "Cond",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    return model;
  }

  auto make_header_widths() {
    auto widths = std::vector<int>();
    widths.push_back(scale_width(45));
    widths.push_back(scale_width(50));
    widths.push_back(scale_width(40));
    widths.push_back(scale_width(38));
    return widths;
  }
}

TimeAndSalesTableView::TimeAndSalesTableView(
    std::shared_ptr<TableModel> table,
    std::shared_ptr<TimeAndSalesWindowProperties> properties, QWidget* parent)
    : m_table(std::move(table)),
      m_properties(std::move(properties))/*,
      m_update_connection(m_time_and_sales->connect_update_signal(
        std::bind_front(&TimeAndSalesTableView::on_update, this)))*/ {
      /*m_current_connection(m_time_and_sales->get_security()->connect_update_signal(
        std::bind_front(&TimeAndSalesTableView::on_current, this))) {*/
  m_table_model = std::make_shared<ArrayTableModel>();
  auto table_view = TableViewBuilder(m_table_model).
    set_header(make_header_model()).
    set_view_builder(
      std::bind_front(&TimeAndSalesTableView::table_view_builder, this)).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
    table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_header->get_widths()->set(i, widths[i]);
  }
  enclose(*this, *table_view);
}

const std::shared_ptr<TableModel>& TimeAndSalesTableView::get_table() const {
  return m_table;
}

const std::shared_ptr<TimeAndSalesWindowProperties>&
  TimeAndSalesTableView::get_properties() const {
  return m_properties;
}

QWidget* TimeAndSalesTableView::table_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  switch(static_cast<Column>(column)) {
  case Column::TIME:
    return make_time_box(table->get<time_duration>(row, column));
    //return make_time_box(make_list_value_model(
    //  std::make_shared<ColumnViewListModel<time_duration>>(table,
    //  column), row)->get());
  case Column::PRICE:
    return new MoneyBox(std::make_shared<ScalarValueModelDecorator<optional<Money>>>(make_list_value_model(
      std::make_shared<ColumnViewListModel<Money>>(table,column), row)));
  case Column::SIZE:
    return new QuantityBox(std::make_shared<ScalarValueModelDecorator<optional<Quantity>>>(
      make_list_value_model(
      std::make_shared<ColumnViewListModel<Quantity>>(
      table, column), row)));
  case Column::MARKET:
    return new MarketBox(make_list_value_model(
      std::make_shared<ColumnViewListModel<MarketCode>>(table,column), row));

  }
  return nullptr;
}

//void TimeAndSalesTableView::query_until(Beam::Queries::Sequence sequence) {
//  auto results = m_time_and_sales->query_until(sequence, 20).then(
//    [=] (std::vector<TimeAndSalesModel::Entry> entries) {
//      for(auto& entry : entries) {
//        auto row = std::vector<std::any>();
//        row.push_back(entry.m_time_and_sale.GetValue().m_timestamp);
//        row.push_back(entry.m_time_and_sale.GetValue().m_price);
//        row.push_back(entry.m_time_and_sale.GetValue().m_size);
//        row.push_back(entry.m_time_and_sale.GetValue().m_marketCenter);
//        row.push_back(entry.m_time_and_sale.GetValue().m_condition);
//        m_table_model->push(row);
//      }
//    });
//}

//void TimeAndSalesTableView::on_current(const Security& security) {
//
//}

//void TimeAndSalesTableView::on_update(const TimeAndSalesModel::Entry& entry) {
//  auto row = std::vector<std::any>();
//  row.push_back(entry.m_time_and_sale.GetValue().m_timestamp);
//  row.push_back(entry.m_time_and_sale.GetValue().m_price);
//  row.push_back(entry.m_time_and_sale.GetValue().m_size);
//  row.push_back(entry.m_time_and_sale.GetValue().m_marketCenter);
//  row.push_back(entry.m_time_and_sale.GetValue().m_condition);
//  m_table_model->insert(row, 0);
//}
