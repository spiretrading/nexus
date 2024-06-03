#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
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
  }

  auto apply_table_header_style(StyleSheet& style) {
    style.get(Any()).
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

  auto make_time_cell(ptime time) {
    auto time_text = to_text(time);
    time_text = time_text.left(time_text.lastIndexOf('.'));
    return make_label(time_text);
  }

  QWidget* table_view_builder(const std::shared_ptr<TableModel>& table, int row,
      int column) {
    auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
    auto cell = [&] () -> QWidget* {
      if(column_id == TimeAndSalesTableModel::Column::TIME) {
        return make_time_cell(table->get<ptime>(row, column));
      } else if(column_id == TimeAndSalesTableModel::Column::PRICE) {
        auto money_cell = make_label(to_text(table->get<Money>(row, column)));
        update_style(*money_cell,
          std::bind_front(&apply_table_cell_right_align_style));
        return money_cell;
      } else if(column_id == TimeAndSalesTableModel::Column::SIZE) {
        auto quantity_cell = make_label(
          to_text(table->get<Quantity>(row, column)).remove(QChar(',')));
        update_style(*quantity_cell,
          std::bind_front(&apply_table_cell_right_align_style));
        return quantity_cell;
      } else if(column_id == TimeAndSalesTableModel::Column::MARKET) {
        return make_label(
          QString::fromStdString(table->get<std::string>(row, column)));
      } else if(column_id == TimeAndSalesTableModel::Column::CONDITION) {
        return make_label(
          to_text(table->get<TimeAndSale::Condition>(row, column)));
      }
      return make_label("");
    }();
    update_style(*cell, std::bind_front(&apply_table_cell_style));
    return cell;
  }
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_view_builder(table_view_builder).make();
  update_style(*table_view, std::bind_front(&apply_table_view_style));
  auto& header = table_view->get_header();
  auto header_scroll_box = new ScrollBox(&header);
  header_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  header_scroll_box->setFocusPolicy(Qt::NoFocus);
  header_scroll_box->set_horizontal(ScrollBox::DisplayPolicy::NEVER);
  header_scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
  update_style(*table_view, std::bind_front(&apply_table_header_style));
  auto& header_box =
    *static_cast<Box*>(table_view->layout()->itemAt(0)->widget());
  auto old_header_box =
    table_view->layout()->replaceWidget(&header_box, header_scroll_box);
  delete old_header_box->widget();
  delete old_header_box;
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
  auto& body_scroll_box =
    *static_cast<ScrollBox*>(table_view->layout()->itemAt(1)->widget());
  body_scroll_box.get_horizontal_scroll_bar().connect_position_signal(
    std::bind_front(&ScrollBar::set_position,
      &header_scroll_box->get_horizontal_scroll_bar()));
  return table_view;
}
