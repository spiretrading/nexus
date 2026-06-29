#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/PullIndicator.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/TableColumnReorderController.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using IndicatorRow = StateSelector<BboIndicator, struct IndicatorRowTag>;
  using ShowGrid = StateSelector<void, struct ShowGridTag>;
  const auto DEBOUNCE_TIME_MS = 100;

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
    add_item(QObject::tr("Buyer"), QObject::tr("Buy"));
    add_item(QObject::tr("Seller"), QObject::tr("Sell"));
    return model;
  }

  struct HeaderItemProperties {
    Qt::Alignment m_alignment;
    int m_width;
  };

  auto make_header_item_properties() {
    auto properties = std::vector<HeaderItemProperties>();
    properties.emplace_back(Qt::AlignLeft, scale_width(48));
    properties.emplace_back(Qt::AlignRight, scale_width(50));
    properties.emplace_back(Qt::AlignRight, scale_width(40));
    properties.emplace_back(Qt::AlignLeft, scale_width(38));
    properties.emplace_back(Qt::AlignLeft, scale_width(34));
    properties.emplace_back(Qt::AlignLeft, scale_width(38));
    properties.emplace_back(Qt::AlignLeft, scale_width(38));
    return properties;
  }

  void make_header_menu(TableView& table_view,
      const std::shared_ptr<TimeAndSalesPropertiesModel>& properties) {
    auto& header = table_view.get_header();
    header.setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(&header, &QWidget::customContextMenuRequested,
      [=, &table_view] (const auto& pos) {
        auto& header = table_view.get_header();
        auto menu = new ContextMenu(header);
        for(auto i = 0; i != TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
          auto is_checked =
            menu->add_check_box(header.get_items()->get(i).m_name);
          is_checked->set(properties->get().is_visible(
            static_cast<TimeAndSalesTableModel::Column>(i)));
          is_checked->connect_update_signal([=, &table_view] (auto checked) {
            auto current_properties = properties->get();
            current_properties.set_visible(
              static_cast<TimeAndSalesTableModel::Column>(i), checked);
            properties->set(current_properties);
          });
        }
        menu->window()->setAttribute(Qt::WA_DeleteOnClose);
        menu->window()->move(header.mapToGlobal(pos));
        menu->window()->show();
      });
  }

  struct ItemBuilder {
    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      if(column >= TimeAndSalesTableModel::COLUMN_SIZE) {
        return new QWidget();
      }
      auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
      auto cell = make_label(QString());
      if(column_id == TimeAndSalesTableModel::Column::PRICE ||
          column_id == TimeAndSalesTableModel::Column::SIZE) {
        update_style(*cell, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
        });
      }
      reset(*cell, table, row, column);
      return cell;
    }

    void reset(QWidget& widget, const std::shared_ptr<TableModel>& table,
        int row, int column) {
      if(column >= TimeAndSalesTableModel::COLUMN_SIZE) {
        return;
      }
      auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
      auto& cell = static_cast<TextBox&>(widget);
      auto& current = *cell.get_current();
      if(column_id == TimeAndSalesTableModel::Column::TIME) {
        auto time = to_text(table->get<ptime>(row, column));
        current.set(time.left(time.lastIndexOf('.')));
      } else if(column_id == TimeAndSalesTableModel::Column::PRICE) {
        current.set(to_text(table->get<Money>(row, column)));
      } else if(column_id == TimeAndSalesTableModel::Column::SIZE) {
        current.set(
          to_text(table->get<Quantity>(row, column)).remove(QChar(',')));
      } else if(column_id == TimeAndSalesTableModel::Column::MARKET) {
        current.set(
          QString::fromStdString(table->get<std::string>(row, column)));
      } else if(column_id == TimeAndSalesTableModel::Column::CONDITION) {
        current.set(to_text(table->get<TimeAndSale::Condition>(row, column)));
      } else {
        current.set(to_text(table->get<std::string>(row, column)));
      }
      for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
        auto indicator = static_cast<BboIndicator>(i);
        unmatch(cell, IndicatorRow(indicator));
      }
      auto time_and_sales_table =
        std::static_pointer_cast<TimeAndSalesTableModel>(table);
      auto indicator = time_and_sales_table->get_bbo_indicator(row);
      match(cell, IndicatorRow(indicator));
    }

    void unmount(QWidget* widget) {
      delete widget;
    }
  };

  struct TableViewStylist : QObject {
    std::shared_ptr<TimeAndSalesPropertiesModel> m_properties;
    std::vector<int> m_last_column_order;
    std::bitset<TimeAndSalesTableModel::COLUMN_SIZE> m_last_visible_columns;
    optional<TimeAndSalesProperties> m_last_styles;
    QTimer m_timer;
    bool m_is_moving;
    scoped_connection m_connection;

    TableViewStylist(TableView& table_view,
        std::shared_ptr<TimeAndSalesPropertiesModel> properties)
        : QObject(&table_view),
          m_properties(std::move(properties)),
          m_timer(this),
          m_is_moving(false) {
      m_timer.setSingleShot(true);
      m_timer.setInterval(DEBOUNCE_TIME_MS);
      connect(&m_timer, &QTimer::timeout, this,
        std::bind_front(&TableViewStylist::on_timeout, this));
      m_last_visible_columns.set();
      m_last_column_order.resize(m_properties->get().get_column_order().size());
      std::iota(m_last_column_order.begin(), m_last_column_order.end(), 0);
      reorder_column_order(m_properties->get());
      update_style(table_view, [&] (auto& style) {
        style.get(ShowGrid() > is_a<TableBody>()).
          set(HorizontalSpacing(scale_width(1))).
          set(VerticalSpacing(scale_height(1)));
      });
      update_style(table_view.get_header(),
        [&] (auto& style) {
          style.get(Any() > is_a<TableHeaderItem>()).
            set(PaddingLeft(scale_width(4))).
            set(PaddingTop(scale_height(5))).
            set(PaddingBottom(scale_height(4)));
        });
      update_style(table_view.get_body(), [&] (auto& style) {
        style.get(Any()).
          set(grid_color(QColor(0xE0E0E0))).
          set(padding(0)).
          set(spacing(0)).
          set(PaddingBottom(PullIndicator::TABLE_BODY_BOTTOM_PADDING()));
        style.get(Any() > Row() > is_a<TableItem>() > is_a<TextBox>()).
          set(border_size(0)).
          set(horizontal_padding(scale_width(2))).
          set(vertical_padding(scale_height(1.5)));
      });
      auto header_properties = make_header_item_properties();
      auto& header = table_view.get_header();
      for(auto i = 0; i < std::ssize(header_properties); ++i) {
        auto index = m_last_column_order[i];
        header.get_widths()->set(i, header_properties[index].m_width);
        update_style(*header.get_item(i), [&] (auto& style) {
          style.get(Any() > TableHeaderItem::Label()).
            set(TextAlign(header_properties[index].m_alignment));
        });
      }
      apply_column_visibility(m_properties->get());
      apply_styles(m_properties->get());
      m_connection = m_properties->connect_update_signal(
        std::bind_front(&TableViewStylist::on_properties, this));
    }

    void apply_column_visibility(const TimeAndSalesProperties& properties) {
      auto& table_view = *static_cast<TableView*>(parent());
      for(auto i = 0; i != TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
        auto is_visible = properties.is_visible(
          static_cast<TimeAndSalesTableModel::Column>(i));
        if(is_visible == m_last_visible_columns[i]) {
          continue;
        }
        m_last_visible_columns[i] = is_visible;
        if(is_visible) {
          table_view.show_column(i);
        } else {
          table_view.hide_column(i);
        }
      }
    }

    void apply_styles(const TimeAndSalesProperties& properties) {
      auto& table_view = *static_cast<TableView*>(parent());
      auto font_changed = !m_last_styles ||
        properties.get_font() != m_last_styles->get_font();
      auto changed_indicators = std::array<BboIndicator, BBO_INDICATOR_COUNT>();
      auto changed_count = 0;
      for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
        auto indicator = static_cast<BboIndicator>(i);
        if(!m_last_styles ||
            properties.get_highlight_color(indicator) !=
              m_last_styles->get_highlight_color(indicator)) {
          changed_indicators[changed_count++] = indicator;
        }
      }
      if(font_changed) {
        update_style(table_view.get_header(), [&] (auto& style) {
          style.get(Any() > is_a<TableHeaderItem>() > TableHeaderItem::Label()).
            set(Font(properties.get_font()));
        });
      }
      if(font_changed || changed_count > 0) {
        update_style(table_view.get_body(), [&] (auto& style) {
          if(font_changed) {
            style.get(Any() > Row() > is_a<TableItem>() > is_a<TextBox>()).
              set(Font(properties.get_font()));
          }
          for(auto i = 0; i < changed_count; ++i) {
            auto indicator = changed_indicators[i];
            auto selector = IndicatorRow(indicator);
            auto highlight = properties.get_highlight_color(indicator);
            style.get(Any() > Row() > is_a<TableItem>() > selector).
              set(TextColor(highlight.m_text_color));
            style.get(Any() > (+Row() > (is_a<TableItem>() > selector))).
              set(BackgroundColor(highlight.m_background_color));
          }
        });
      }
      if(!m_last_styles ||
          properties.is_grid_enabled() != m_last_styles->is_grid_enabled()) {
        if(properties.is_grid_enabled()) {
          match(table_view, ShowGrid());
        } else {
          unmatch(table_view, ShowGrid());
        }
      }
      m_last_styles = properties;
    }

    void reorder_column_order(const TimeAndSalesProperties& properties) {
      auto& header = static_cast<TableView*>(parent())->get_header();
      auto& order = properties.get_column_order();
      for(int i = 0; i < order.size(); ++i) {
        if(m_last_column_order[i] == order[i]) {
          continue;
        }
        if(auto iter = std::find(m_last_column_order.begin() + i,
            m_last_column_order.end(), order[i]);
            iter != m_last_column_order.end()) {
          auto from = static_cast<int>(iter - m_last_column_order.begin()) ;
          header.get_items()->move(from, i);
          auto value = m_last_column_order[from];
          m_last_column_order.erase(m_last_column_order.begin() + from);
          m_last_column_order.insert(m_last_column_order.begin() + i, value);
        }
      }
    }

    void on_properties(const TimeAndSalesProperties& properties) {
      if(m_is_moving) {
        auto& column_order = properties.get_column_order();
        m_last_column_order.assign(column_order.begin(), column_order.end());
      } else {
        reorder_column_order(properties);
      }
      apply_column_visibility(properties);
      if(!m_timer.isActive()) {
        m_timer.start();
      }
    }

    void on_timeout() {
      apply_styles(m_properties->get());
    }
  };
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table,
    std::shared_ptr<TimeAndSalesPropertiesModel> properties, QWidget* parent) {
  auto builder = ItemBuilder();
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(RecycledTableViewItemBuilder(builder)).
    set_current(
      std::make_shared<ConstantValueModel<optional<TableIndex>>>(none)).
    make();
  make_header_menu(*table_view, properties);
  auto pull_indicator = new PullIndicator(*table_view);
  auto stylist = new TableViewStylist(*table_view, properties);
  auto& column_order = properties->get().get_column_order();
  auto controller = new TableColumnReorderController(*table_view, builder,
    std::vector<int>(column_order.begin(), column_order.end()));
  controller->connect_column_moved_signal([=] (int source, int destination) {
    auto current_properties = properties->get();
    current_properties.move_column(
      static_cast<TimeAndSalesTableModel::Column>(source),
      static_cast<TimeAndSalesTableModel::Column>(destination));
    stylist->m_is_moving = true;
    properties->set(current_properties);
    stylist->m_is_moving = false;
  });
  return table_view;
}
