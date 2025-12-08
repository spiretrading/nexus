#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QGraphicsOpacityEffect>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/PullIndicator.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
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
    add_item("", "");
    return model;
  }

  auto make_header_model(const ListModel<TableHeaderItem::Model>& model, int column) {
    auto one_column_model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    one_column_model->push(model.get(column));
    one_column_model->push({"", "", TableHeaderItem::Order::UNORDERED,
      TableFilter::Filter::NONE});
    return one_column_model;
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

  struct ColumnViewTableModel : TableModel {
    std::shared_ptr<TableModel> m_source;
    int m_column;

    ColumnViewTableModel(std::shared_ptr<TableModel> source, int column)
      : m_source(std::move(source)),
        m_column(column) {}

    int get_row_size() const override {
      return m_source->get_row_size();
    }

    int get_column_size() const override {
      return 1;
    }

    AnyRef at(int row, int column) const override {
      if(column != 0) {
        throw std::out_of_range("The column is out of range.");
      }
      return m_source->at(row, m_column);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_source->connect_operation_signal(slot);
    }
  };

  struct ColumnTableViewItemBuilder {
    TableViewItemBuilder m_builder;
    std::shared_ptr<TableModel> m_source;
    int m_column;

    ColumnTableViewItemBuilder(TableViewItemBuilder builder,
      std::shared_ptr<TableModel> source, int column)
      : m_builder(std::move(builder)),
        m_source(std::move(source)),
        m_column(column) {}

    QWidget* mount(const std::shared_ptr<TableModel>& table, int row,
        int column) {
      return m_builder.mount(m_source, row, m_column);
    }

    void unmount(QWidget* widget) {
      m_builder.unmount(widget);
    }
  };

  QWidget* make_column_cover(TableView& table_view, int index) {
    auto cover = new Box(nullptr, &table_view);
    update_style(*cover, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(BorderLeftSize(scale_width(2))).
        set(BorderLeftColor(QColor(0x4B23A0)));
    });
    cover->show();
    return cover;
  }

  QWidget* make_column_preview(TableView& table_view,
      const TableViewItemBuilder& m_item_builder,
      int visual_index, int logical_index) {
    auto& table_header = table_view.get_header();
    table_header.grabMouse();
    auto item = table_header.get_item(visual_index);
    auto table_view_preview = TableViewBuilder(
      std::make_shared<ColumnViewTableModel>(
        table_view.get_table(), logical_index)).
      set_header(make_header_model(*table_header.get_items(), visual_index)).
      set_item_builder(ColumnTableViewItemBuilder(
        m_item_builder, table_view.get_table(), logical_index)).
      set_current(
        std::make_shared<ConstantValueModel<optional<TableIndex>>>(none)).
      make();
    table_view_preview->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
    auto width = table_header.get_widths()->get(visual_index);
    auto& float_table_header = table_view_preview->get_header();
    float_table_header.get_widths()->set(0, width);
    set_style(*table_view_preview, get_style(table_view));
    set_style(float_table_header, get_style(table_header));
    set_style(*float_table_header.get_item(0), get_style(*item));
    set_style(table_view_preview->get_body(), get_style(table_view.get_body()));
    auto preview = new Box(table_view_preview, table_view.window());
    auto effect = new QGraphicsOpacityEffect(preview);
    effect->setOpacity(0.5);
    preview->setGraphicsEffect(effect);
    update_style(*preview, [] (auto& style) {
      style.get(Any()).
        set(BorderTopSize(scale_height(1))).
        set(BorderLeftSize(scale_width(1))).
        set(BorderRightSize(scale_width(1))).
        set(border_color(QColor(0x4B23A0)));
    });
    preview->show();
    if(is_match(table_view, ShowGrid())) {
      match(*table_view_preview, ShowGrid());
    }
    auto& vertical_scroll_bar =
      table_view.get_scroll_box().get_vertical_scroll_bar();
    auto& float_vertical_scroll_bar =
      table_view_preview->get_scroll_box().get_vertical_scroll_bar();
    float_vertical_scroll_bar.set_range(vertical_scroll_bar.get_range());
    float_vertical_scroll_bar.set_position(vertical_scroll_bar.get_position());
    return preview;
  }

  template <typename Widths>
  std::tuple<bool, int, int> find_column_at_position(TableHeader& header,
      const Widths& widths, int x) {
    auto last_visible_column = -1;
    auto left = 0;
    auto index = 0;
    for(auto i = std::begin(widths); i != std::end(widths); ++i, ++index) {
      auto item = header.get_item(index);
      if(!item->isVisible()) {
        continue;
      }
      last_visible_column = index;
      auto right = left + *i;
      if(x >= left && x < right) {
        return {true, index, left};
      }
      left = right;
    }
    return {false, last_visible_column, left};
  }

  struct TableViewColumnMover : QObject {
    using ColumnMoved = Signal<void(int source, int destination)>;
    mutable ColumnMoved m_column_moved_signal;
    TableView* m_table_view;
    TableViewItemBuilder m_item_builder;
    QWidget* m_column_preview;
    QWidget* m_column_cover;
    QWidget* m_horizontal_scroll_bar_parent;
    QWidget* m_vertical_scroll_bar_parent;
    int m_last_mouse_x;
    int m_left_padding;
    int m_source_index;
    int m_current_index;
    int m_item_x_offset;
    std::vector<int> m_visual_to_logical;
    std::vector<int> m_widths;

    TableViewColumnMover(TableView& table_view,
        TableViewItemBuilder item_builder)
        : QObject(&table_view),
          m_table_view(&table_view),
          m_item_builder(std::move(item_builder)),
          m_column_preview(nullptr),
          m_column_cover(nullptr),
          m_horizontal_scroll_bar_parent(nullptr),
          m_vertical_scroll_bar_parent(nullptr),
          m_last_mouse_x(0),
          m_left_padding(0),
          m_source_index(-1),
          m_current_index(-1),
          m_item_x_offset(0) {
      auto& header = m_table_view->get_header();
      header.setMouseTracking(true);
      header.installEventFilter(this);
      m_visual_to_logical.resize(header.get_items()->get_size());
      std::iota(m_visual_to_logical.begin(), m_visual_to_logical.end(), 0);
      header.setCursor(Qt::OpenHandCursor);
    }

    connection connect_column_moved_signal(
        const ColumnMoved::slot_type & slot) const {
      return m_column_moved_signal.connect(slot);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_table_view->get_header().get_item(m_current_index) &&
          m_column_cover && event->type() == QEvent::Move) {
        move_column_cover(m_current_index);
      } else if(watched == &m_table_view->get_header()) {
        auto& header = m_table_view->get_header();
        if(event->type() == QEvent::MouseButtonPress) {
          auto& mouse_event = *static_cast<QMouseEvent*>(event);
          if(mouse_event.button() == Qt::LeftButton) {
            if(auto [is_found, index, item_left] = find_column_at_position(
                header, *header.get_widths(), mouse_event.x());
                is_found) {
              m_item_x_offset = mouse_event.x() - item_left;
              start_drag(index);
            }
          }
        } else if(event->type() == QEvent::MouseButtonRelease) {
          stop_drag();
        } else if(event->type() == QEvent::MouseMove) {
          drag_move(*static_cast<QMouseEvent*>(event));
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void start_drag(int index) {
      if(m_column_preview) {
        return;
      }
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
      auto& header = m_table_view->get_header();
      header.grabMouse();
      auto& header_stylist = find_stylist(header);
      auto header_proxies = header_stylist.get_proxies();
      for(auto& property : header_proxies[0]->get_computed_block()) {
        property.visit(
          [&] (const PaddingLeft& padding) {
            header_stylist.evaluate(padding, [&] (auto padding) {
              m_left_padding = padding;
            });
          });
      }
      auto item = header.get_item(index);
      item->installEventFilter(this);
      m_column_cover = make_column_cover(*m_table_view, index);
      m_column_cover->setFixedSize(
        header.get_widths()->get(index) + m_left_padding,
        m_table_view->height());
      move_column_cover(index);
      m_column_preview = make_column_preview(*m_table_view, m_item_builder,
        index, m_visual_to_logical[index]);
      m_column_preview->setAttribute(Qt::WA_TransparentForMouseEvents);
      m_column_preview->setFixedSize(m_column_cover->width() + scale_width(1),
        m_column_cover->height() + scale_height(1));
      auto position =
        m_column_cover->mapTo(m_column_preview->parentWidget(), QPoint(0, 0));
      m_column_preview->move(position.x() - scale_width(1),
        position.y() - scale_height(1));
      auto& horizontal_scroll_bar =
        m_table_view->get_scroll_box().get_horizontal_scroll_bar();
      auto horizontal_scroll_bar_position =
        horizontal_scroll_bar.mapToGlobal(QPoint(0, 0));
      if(horizontal_scroll_bar.isVisible()) {
        m_horizontal_scroll_bar_parent =
          raise_scroll_bar(horizontal_scroll_bar);
      }
      auto& vertical_scroll_bar =
        m_table_view->get_scroll_box().get_vertical_scroll_bar();
      auto vertical_scroll_bar_position =
        vertical_scroll_bar.mapToGlobal(QPoint(0, 0));
      if(vertical_scroll_bar.isVisible()) {
        m_vertical_scroll_bar_parent =
          raise_scroll_bar(vertical_scroll_bar);
      }
      if(horizontal_scroll_bar.isVisible()) {
        horizontal_scroll_bar.move(
          vertical_scroll_bar.parentWidget()->mapFromGlobal(
            horizontal_scroll_bar_position));
      }
      if(vertical_scroll_bar.isVisible()) {
        vertical_scroll_bar.move(
          vertical_scroll_bar.parentWidget()->mapFromGlobal(
            vertical_scroll_bar_position));
      }
      m_widths.assign(header.get_widths()->begin(), header.get_widths()->end());
      m_source_index = index;
      m_current_index = index;
    }

    void stop_drag() {
      QApplication::restoreOverrideCursor();
      m_widths.clear();
      if(m_column_preview) {
        delete m_column_preview;
        m_column_preview = nullptr;
      }
      if(m_column_cover) {
        delete m_column_cover;
        m_column_cover = nullptr;
      }
      if(m_horizontal_scroll_bar_parent) {
        restore_scroll_bar(
          m_table_view->get_scroll_box().get_horizontal_scroll_bar(),
          m_horizontal_scroll_bar_parent, 1, 0);
        m_horizontal_scroll_bar_parent = nullptr;
      }
      if(m_vertical_scroll_bar_parent) {
        restore_scroll_bar(
          m_table_view->get_scroll_box().get_vertical_scroll_bar(),
          m_vertical_scroll_bar_parent, 0, 1);
        m_vertical_scroll_bar_parent = nullptr;
      }
      auto& header = m_table_view->get_header();
      header.releaseMouse();
      if(m_current_index >= 0) {
        header.get_item(m_current_index)->removeEventFilter(this);
        m_column_moved_signal(m_source_index, m_current_index);
        m_source_index = -1;
        m_current_index = -1;
      }
    }

    void drag_move(const QMouseEvent& mouse_event) {
      if(!m_column_preview) {
        return;
      }
      if(m_last_mouse_x == mouse_event.pos().x()) {
        return;
      }
      m_last_mouse_x = mouse_event.pos().x();
      auto start = std::chrono::high_resolution_clock::now();
      auto& horizontal_scroll_bar =
        m_table_view->get_scroll_box().get_horizontal_scroll_bar();
      auto mouse_x = std::max(0, mouse_event.x());
      auto& header = m_table_view->get_header();
      auto [is_found, index, item_left] =
        find_column_at_position(header, m_widths, mouse_x);
      auto should_move_column = [&] {
        if(is_found) {
          if(index != m_current_index) {
            return true;
          }
        } else if(mouse_x >= item_left) {
          return true;
        }
        return false;
      }();
      if(should_move_column) {
        header.get_items()->move(m_current_index, index);
        move_element(m_visual_to_logical, m_current_index, index);
        m_current_index = index;
      }
      if(horizontal_scroll_bar.isVisible()) {
        update_scroll(mouse_x);
      }
      auto x = std::max(0,
        std::min(m_table_view->width() - m_column_preview->width(),
          m_table_view->mapFromGlobal(mouse_event.globalPos()).x()
            - m_item_x_offset));
      m_column_preview->move(x, m_column_preview->y());
    }

    void move_column_cover(int index) {
      auto& item = *m_table_view->get_header().get_item(index);
      m_column_cover->move(
        item.mapTo(m_table_view, QPoint(0, 0)).x() - m_left_padding, 0);
    }

    QWidget* raise_scroll_bar(ScrollBar& scroll_bar) {
      auto parent = scroll_bar.parentWidget();
      auto layout = parent->layout();
      layout->removeWidget(&scroll_bar);
      scroll_bar.setParent(m_column_preview->parentWidget());
      scroll_bar.show();
      return parent;
    }

    void restore_scroll_bar(ScrollBar& scroll_bar, QWidget* original_parent,
        int row, int column) {
      static_cast<QGridLayout*>(original_parent->layout())->addWidget(
        &scroll_bar, row, column);
    }

    void update_scroll(int mouse_x) {
      auto float_table_view_x = std::max(0, mouse_x - m_item_x_offset);
      auto float_table_view_right =
        float_table_view_x + m_column_preview->width();
      auto& horizontal_scroll_bar =
        m_table_view->get_scroll_box().get_horizontal_scroll_bar();
      auto scroll_position = horizontal_scroll_bar.get_position();
      auto scroll_right =
        scroll_position + horizontal_scroll_bar.get_page_size();
      if(float_table_view_x < scroll_position) {
        horizontal_scroll_bar.set_position(float_table_view_x);
      } else if(float_table_view_right >= scroll_right) {
        horizontal_scroll_bar.set_position(std::min(
            horizontal_scroll_bar.get_range().m_end,
            float_table_view_right - horizontal_scroll_bar.get_page_size()));
      }
      if(m_column_cover) {
        move_column_cover(m_current_index);
      }
    }
  };

  struct TableViewStylist : QObject {
    std::shared_ptr<TimeAndSalesPropertiesModel> m_properties;
    scoped_connection m_connection;

    TableViewStylist(TableView& table_view,
        std::shared_ptr<TimeAndSalesPropertiesModel> properties)
        : QObject(&table_view),
          m_properties(std::move(properties)) {
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
        header.get_widths()->set(i, header_properties[i].m_width);
        update_style(*header.get_item(i), [&] (auto& style) {
          style.get(Any() > TableHeaderItem::Label()).
            set(TextAlign(header_properties[i].m_alignment));
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
        if(properties.is_visible(
            static_cast<TimeAndSalesTableModel::Column>(i))) {
          table_view.show_column(i);
        } else {
          table_view.hide_column(i);
        }
      }
    }

    void apply_styles(const TimeAndSalesProperties& properties) {
      auto& table_view = *static_cast<TableView*>(parent());
      update_style(table_view.get_header(), [&] (auto& style) {
        style.get(Any() > is_a<TableHeaderItem>() > TableHeaderItem::Label()).
          set(Font(properties.get_font()));
      });
      update_style(table_view.get_body(), [&] (auto& style) {
        style.get(Any() > Row() > is_a<TableItem>() > is_a<TextBox>()).
          set(Font(properties.get_font()));
        for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
          auto indicator = static_cast<BboIndicator>(i);
          auto selector = IndicatorRow(indicator);
          auto highlight = properties.get_highlight_color(indicator);
          style.get(Any() > Row() > is_a<TableItem>() > selector).
            set(TextColor(highlight.m_text_color));
          style.get(Any() > (+Row() > (is_a<TableItem>() > selector))).
            set(BackgroundColor(highlight.m_background_color));
        }
      });
      if(properties.is_grid_enabled()) {
        match(table_view, ShowGrid());
      } else {
        unmatch(table_view, ShowGrid());
      }
    }

    void on_properties(const TimeAndSalesProperties& properties) {
      apply_column_visibility(properties);
      const auto DEBOUNCE_TIME_MS = 100;
      QTimer::singleShot(DEBOUNCE_TIME_MS, this, [=] {
        apply_styles(properties);
      });
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
  auto column_move = new TableViewColumnMover(*table_view, builder);
  column_move->connect_column_moved_signal([=] (int source, int destination) {
    auto current_properties = properties->get();
    current_properties.move_column(
      static_cast<TimeAndSalesTableModel::Column>(source),
      static_cast<TimeAndSalesTableModel::Column>(destination));
    properties->set(current_properties);
  });
  return table_view;
}
