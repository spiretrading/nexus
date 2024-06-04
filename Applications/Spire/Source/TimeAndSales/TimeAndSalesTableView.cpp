#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QLayout>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct HeaderItemProperties {
    bool m_is_visible;
    Qt::Alignment m_alignment;
    int m_width;
  };

  auto apply_table_view_style(StyleSheet& style) {
    auto body_selector = Any() > is_a<TableBody>();
    style.get(body_selector).
      set(grid_color(Qt::transparent)).
      set(horizontal_padding(0)).
      set(vertical_padding(0)).
      set(HorizontalSpacing(0)).
      set(VerticalSpacing(0));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > CurrentRow()).
      set(BackgroundColor(Qt::transparent));
    style.get(body_selector > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    style.get(Any() > (is_a<TableHeader>() < is_a<Box>())).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
  }

  auto apply_table_cell_style(StyleSheet& style) {
    style.get(Any()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(1.5)));
  }

  auto apply_table_cell_right_align_style(StyleSheet& style) {
    style.get(Any()).set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto push = [&] (const QString& name, const QString& short_name) {
      model->push({name, short_name, TableHeaderItem::Order::UNORDERED,
        TableFilter::Filter::NONE});
    };
    push(QObject::tr("Time"), QObject::tr("Time"));
    push(QObject::tr("Price"), QObject::tr("Px"));
    push(QObject::tr("Size"), QObject::tr("Sz"));
    push(QObject::tr("Market"), QObject::tr("Mkt"));
    push(QObject::tr("Condition"), QObject::tr("Cond"));
    push("", "");
    return model;
  }

  auto make_header_item_properties() {
    auto properties = std::vector<HeaderItemProperties>();
    properties.emplace_back(false, Qt::AlignLeft, scale_width(48));
    properties.emplace_back(true, Qt::AlignRight, scale_width(50));
    properties.emplace_back(true, Qt::AlignRight, scale_width(40));
    properties.emplace_back(true, Qt::AlignLeft, scale_width(38));
    properties.emplace_back(false, Qt::AlignLeft, scale_width(34));
    return properties;
  }

  QWidget* table_view_builder(
      const std::shared_ptr<TimeAndSalesTableModel>& time_and_sales,
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
    auto cell = [&] () -> QWidget* {
      if(column_id == TimeAndSalesTableModel::Column::TIME) {
        auto time = to_text(table->get<ptime>(row, column));
        return make_label(time.left(time.lastIndexOf('.')));
      } else if(column_id == TimeAndSalesTableModel::Column::PRICE) {
        auto money_cell = make_label(to_text(table->get<Money>(row, column)));
        update_style(*money_cell, apply_table_cell_right_align_style);
        return money_cell;
      } else if(column_id == TimeAndSalesTableModel::Column::SIZE) {
        auto quantity_cell = make_label(
          to_text(table->get<Quantity>(row, column)).remove(QChar(',')));
        update_style(*quantity_cell, apply_table_cell_right_align_style);
        return quantity_cell;
      } else if(column_id == TimeAndSalesTableModel::Column::MARKET) {
        return make_label(
          QString::fromStdString(table->get<std::string>(row, column)));
      } else if(column_id == TimeAndSalesTableModel::Column::CONDITION) {
        return make_label(
          to_text(table->get<TimeAndSale::Condition>(row, column)));
      }
      return new QWidget();
    }();
    auto indicator = time_and_sales->get_bbo_indicator(row);
    if(indicator == BboIndicator::UNKNOWN) {
      match(*cell, UnknownIndicator());
    } else if(indicator == BboIndicator::ABOVE_ASK) {
      match(*cell, AboveAskIndicator());
    } else if(indicator == BboIndicator::AT_ASK) {
      match(*cell, AtAskIndicator());
    } else if(indicator == BboIndicator::INSIDE) {
      match(*cell, InsideIndicator());
    } else if(indicator == BboIndicator::AT_BID) {
      match(*cell, AtBidIndicator());
    } else if(indicator == BboIndicator::BELOW_BID) {
      match(*cell, BelowBidIndicator());
    }
    update_style(*cell, apply_table_cell_style);
    return cell;
  }
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_view_builder(std::bind_front(&table_view_builder, table)).make();
  update_style(*table_view, apply_table_view_style);
  auto& header = table_view->get_header();
  auto properties = make_header_item_properties();
  for(auto i = 0; i < std::ssize(properties); ++i) {
    header.get_widths()->set(i, properties[i].m_width);
    auto item = table_view->get_header().get_item(i);
    item->setVisible(properties[i].m_is_visible);
    auto item_layout = item->layout();
    item_layout->setContentsMargins({scale_width(4), scale_height(5), 0, 0});
    auto contents_layout =
      item_layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
    contents_layout->setContentsMargins({});
    if(properties[i].m_alignment == Qt::AlignRight) {
      static_cast<QSpacerItem*>(contents_layout->itemAt(1))->changeSize(0, 0);
      contents_layout->itemAt(2)->widget()->setFixedWidth(0);
      contents_layout->itemAt(3)->widget()->setFixedWidth(0);
      update_style(*item, [] (auto& style) {
        style.get(Any() > TableHeaderItem::Label()).
          set(TextAlign(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter)));
      });
    }
  }
  return table_view;
}
