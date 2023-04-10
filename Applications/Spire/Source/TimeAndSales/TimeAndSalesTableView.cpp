#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/SaleConditionBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto TABLE_VIEW_STYLE(StyleSheet style) {
    style.get(Any() > is_a<TableBody>()).
      set(horizontal_padding(0));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    return style;
  }

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
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent)
    : m_table(std::move(table)) {
  auto table_view = TableViewBuilder(m_table).
    set_header(make_header_model()).
    set_view_builder(
      std::bind_front(&TimeAndSalesTableView::table_view_builder, this)).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*table_view, [] (auto& style) {
    style = TABLE_VIEW_STYLE(style);
  });
  auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
    table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_header->get_widths()->set(i, widths[i]);
  }
  enclose(*this, *table_view);
  m_table_header = static_cast<TableHeader*>(static_cast<Box*>(
    table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto& scroll_box =
    *static_cast<ScrollBox*>(table_view->layout()->itemAt(1)->widget());
  scroll_box.setFocusPolicy(Qt::NoFocus);
  m_table_body = static_cast<TableBody*>(&scroll_box.get_body());
}

const std::shared_ptr<TimeAndSalesTableModel>& TimeAndSalesTableView::get_table() const {
  return m_table;
}

const TableItem* TimeAndSalesTableView::get_item(Index index) const {
  return m_table_body->get_item(index);
}

TableItem* TimeAndSalesTableView::get_item(Index index) {
  return m_table_body->get_item(index);
}

QWidget* TimeAndSalesTableView::table_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  switch(static_cast<Column>(column)) {
    case Column::TIME:
    {
      auto time_box = make_time_box(table->get<ptime>(row, column).time_of_day());
      time_box->set_read_only(true);
      update_style(*time_box, [] (auto& style) {
        style.get(Any()).
          set(horizontal_padding(scale_width(1))).
          set(vertical_padding(scale_height(1)));
        style.get(Any() > is_a<DecimalBox>()).set(TrailingZeros(0));
      });
      return time_box;
    }
    case Column::PRICE:
    {
      auto modifiers = QHash<Qt::KeyboardModifier, Money>(
        {{Qt::NoModifier, Money::ONE}, {Qt::AltModifier, 5 * Money::ONE},
        {Qt::ControlModifier, 10 * Money::ONE}, {Qt::ShiftModifier, 20 * Money::ONE}});
      auto money_box = new MoneyBox(std::make_shared<LocalOptionalMoneyModel>(table->get<Money>(row, column)),
        std::move(modifiers));
      update_style(*money_box, [] (auto& style) {
        style.get(Any()).
          set(horizontal_padding(scale_width(2))).
          set(vertical_padding(scale_height(2)));
      });
      money_box->set_read_only(true);
      return money_box;
    }
    case Column::SIZE:
    {
       auto modifiers = QHash<Qt::KeyboardModifier, Quantity>(
        {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
          {Qt::ShiftModifier, 20}});
      auto quantity_box = new QuantityBox(std::make_shared<LocalOptionalQuantityModel>(table->get<Quantity>(row, column)),
        std::move(modifiers));
      update_style(*quantity_box, [] (auto& style) {
        style.get(Any()).
          set(horizontal_padding(scale_width(2))).
          set(vertical_padding(scale_height(2)));
      });
      quantity_box->set_read_only(true);
      return quantity_box;
    }
    case Column::MARKET:
    {
      auto market_code = table->get<std::string>(row, column);
      auto query_model = std::make_shared<LocalComboBoxQueryModel>();
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      query_model->add(displayText(MarketToken(market.m_code)).toLower(), market);
      query_model->add(QString(market.m_code.GetData()).toLower(), market);
      auto market_box = new MarketBox(std::move(query_model),
        std::make_shared<LocalValueModel<MarketCode>>(market_code));
      update_style(*market_box, [] (auto& style) {
        style.get(ReadOnly()).
          set(horizontal_padding(scale_width(2))).
          set(vertical_padding(scale_height(2)));
      });
      market_box->set_read_only(true);
      return market_box;
    }
    case Column::CONDITION:
    {
      auto condition_box = new SaleConditionBox(std::make_shared<LocalComboBoxQueryModel>(),
        make_list_value_model(
        std::make_shared<ColumnViewListModel<TimeAndSale::Condition>>(table, column), row));
      update_style(*condition_box, [] (auto& style) {
        style.get(Any()).
          set(horizontal_padding(scale_width(2))).
          set(vertical_padding(scale_height(2)));
      });
      condition_box->set_read_only(true);
      return condition_box;
    }
  }
  return nullptr;
}
