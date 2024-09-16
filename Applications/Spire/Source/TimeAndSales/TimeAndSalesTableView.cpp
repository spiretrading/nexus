#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QLayout>
#include <QMovie>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ContextMenu.hpp"
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
  const auto CELL_VERTICAL_PADDING = 1.5;
  const auto PULL_DELAY_TIMEOUT_MS = 1000;

  struct HeaderItemProperties {
    bool m_is_visible;
    Qt::Alignment m_alignment;
    int m_width;
  };

  struct Status {
    bool m_is_loading;
    int m_last_scroll_y;
  };

  void apply_indicator_style(StyleSheet& style, const Selector& item_selector,
      const Selector& indicator_selector, const QColor& background_color,
      const QColor& text_color) {
    style.get(item_selector > (indicator_selector < is_a<TableItem>() < Row())).
      set(BackgroundColor(background_color));
    style.get(item_selector > indicator_selector).
      set(TextColor(text_color));
  };

  auto apply_table_view_style(StyleSheet& style) {
    auto body_selector = Any() > is_a<TableBody>();
    auto body_item_selector = body_selector > Row() > is_a<TableItem>();
    auto header_selector = Any() > is_a<TableHeader>();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    style.get(header_selector > is_a<TableHeaderItem>()).
      set(PaddingLeft(scale_width(4))).
      set(PaddingTop(scale_height(5))).
      set(PaddingBottom(scale_height(4)));
    style.get(
        header_selector > is_a<TableHeaderItem>() > TableHeaderItem::Label()).
      set(Font(font));
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
    style.get(Any() > is_a<TableBody>()).
      set(PaddingBottom(scale_height(44)));
    apply_indicator_style(style, body_item_selector, AboveAskIndicator(),
      QColor(0xEBFFF0), QColor(0x007735));
    apply_indicator_style(style, body_item_selector, AtAskIndicator(),
      QColor(0xEBFFF0), QColor(0x007735));
    apply_indicator_style(style, body_item_selector, InsideIndicator(),
      QColor(0xFFFFFF), QColor(Qt::black));
    apply_indicator_style(style, body_item_selector, AtBidIndicator(),
      QColor(0xFFF1F1), QColor(0xB71C1C));
    apply_indicator_style(style, body_item_selector, BelowBidIndicator(),
      QColor(0xFFF1F1), QColor(0xB71C1C));
    style.get(body_item_selector > is_a<TextBox>()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(CELL_VERTICAL_PADDING))).
      set(Font(font));
  }

  auto apply_table_cell_right_align_style(StyleSheet& style) {
    style.get(Any()).set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto add_item = [&] (const QString& name, const QString& short_name) {
      model->push({name, short_name, TableHeaderItem::Order::UNORDERED,
        TableFilter::Filter::NONE});
    };
    add_item(QObject::tr("Time"), QObject::tr("Time"));
    add_item(QObject::tr("Price"), QObject::tr("Px"));
    add_item(QObject::tr("Size"), QObject::tr("Sz"));
    add_item(QObject::tr("Market"), QObject::tr("Mkt"));
    add_item(QObject::tr("Condition"), QObject::tr("Cond"));
    add_item("", "");
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

  QWidget* item_builder(
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
    return cell;
  }
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(std::bind_front(&item_builder, table)).make();
  update_style(*table_view, apply_table_view_style);
  auto& header = table_view->get_header();
  auto properties = make_header_item_properties();
  auto table_header_menu = new ContextMenu(header);
  for(auto i = 0; i < std::ssize(properties); ++i) {
    header.get_widths()->set(i, properties[i].m_width);
    auto item = header.get_item(i);
    item->setVisible(properties[i].m_is_visible);
    update_style(*item, [&] (auto& style) {
      style.get(Any() > TableHeaderItem::Label()).
        set(TextAlign(properties[i].m_alignment));
    });
    auto model =
      table_header_menu->add_check_box(item->get_model()->get().m_name);
    model->set(properties[i].m_is_visible);
    model->connect_update_signal([=] (auto checked) {
      table_view->get_header().get_item(i)->setVisible(checked);
    });
  }
  header.setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(&header, &QWidget::customContextMenuRequested,
    [=] (const auto& pos) {
      table_header_menu->window()->move(
        table_view->get_header().mapToGlobal(pos));
      table_header_menu->window()->show();
    });
  auto scroll_box = &table_view->get_scroll_box();
  auto status = std::make_shared<Status>(false, 0);
  scroll_box->get_vertical_scroll_bar().connect_position_signal(
    [=] (int position) {
      auto& scroll_bar = scroll_box->get_vertical_scroll_bar();
      if(!status->m_is_loading && position > status->m_last_scroll_y &&
          scroll_bar.get_range().m_end - position <
            scroll_bar.get_page_size() / 2) {
        table->load_history(scroll_box->height() /
          table_view->get_body().estimate_scroll_line_height());
      }
      status->m_last_scroll_y = position;
    });
  table->connect_begin_loading_signal([=] {
    if(status->m_is_loading) {
      return;
    }
    status->m_is_loading = true;
    QTimer::singleShot(PULL_DELAY_TIMEOUT_MS, table_view, [=] {
      if(!status->m_is_loading) {
        return;
      }
      scroll_to_end(scroll_box->get_vertical_scroll_bar());
    });
  });
  table->connect_end_loading_signal([=] {
    status->m_is_loading = false;
  });
  return table_view;
}
