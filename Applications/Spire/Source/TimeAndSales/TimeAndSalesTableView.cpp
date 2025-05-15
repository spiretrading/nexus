#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QMovie>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
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

  auto TABLE_BODY_BOTTOM_PADDING() {
    static auto height = scale_height(44);
    return height;
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
    add_item(QObject::tr("Buyer"), QObject::tr("Buy"));
    add_item(QObject::tr("Seller"), QObject::tr("Sell"));
    add_item("", "");
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

  struct PullIndicator : QWidget {
    TableView* m_table_view;
    QMovie* m_spinner;
    QTimer m_timer;
    bool m_is_loading;
    int m_last_position;

    PullIndicator(TableView& table_view)
        : QWidget(&table_view.get_body()),
          m_table_view(&table_view),
          m_timer(this),
          m_is_loading(false),
          m_last_position(0) {
      m_spinner = new QMovie(":/Icons/spinner.gif");
      m_spinner->setScaledSize(scale(16, 16));
      auto spinner_widget = new QLabel();
      spinner_widget->setMovie(m_spinner);
      auto box = new Box(spinner_widget);
      enclose(*this, *box);
      proxy_style(*this, *box);
      update_style(*this, [] (auto& style) {
        style.get(Any()).
          set(BodyAlign(Qt::AlignHCenter)).
          set(horizontal_padding(scale_width(8))).
          set(PaddingBottom(scale_height(20))).
          set(PaddingTop(scale_height(8)));
      });
      auto& scroll_box = m_table_view->get_scroll_box();
      scroll_box.installEventFilter(this);
      scroll_box.get_vertical_scroll_bar().connect_position_signal(
        std::bind_front(&PullIndicator::on_position, this));
      auto table =
        std::static_pointer_cast<TimeAndSalesTableModel>(
          m_table_view->get_table());
      table->connect_begin_loading_signal(
        std::bind_front(&PullIndicator::on_begin_loading, this));
      table->connect_end_loading_signal(
        std::bind_front(&PullIndicator::on_end_loading, this));
      m_table_view->get_body().installEventFilter(this);
      hide();
      m_timer.setSingleShot(true);
      connect(&m_timer, &QTimer::timeout,
        std::bind_front(&PullIndicator::on_timeout, this));
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == &m_table_view->get_scroll_box()) {
        if(event->type() == QEvent::KeyPress) {
          auto& key_event = *static_cast<QKeyEvent*>(event);
          if(key_event.key() == Qt::Key_PageUp ||
              key_event.key() == Qt::Key_PageDown) {
            key_event.accept();
            return QCoreApplication::sendEvent(m_table_view, &key_event);
          }
        }
      } else if(event->type() == QEvent::Resize && isVisible()) {
        auto& resize_event = *static_cast<QResizeEvent*>(event);
        update_position(resize_event.size());
      }
      return QObject::eventFilter(watched, event);
    }

    void update_position(const QSize& size) {
      setGeometry(0, size.height() - TABLE_BODY_BOTTOM_PADDING(),
        size.width(), TABLE_BODY_BOTTOM_PADDING());
    }

    void display() {
      auto& scroll_bar =
        m_table_view->get_scroll_box().get_vertical_scroll_bar();
      auto& body = m_table_view->get_body();
      if(!isVisible() && scroll_bar.get_position() >=
          body.sizeHint().height() - scroll_bar.get_page_size()) {
        update_position(body.sizeHint());
        show();
      }
    }

    void on_position(int position) {
      auto& scroll_box = m_table_view->get_scroll_box();
      auto& scroll_bar = scroll_box.get_vertical_scroll_bar();
      if(m_is_loading) {
        if(m_spinner->state() == QMovie::Running) {
          display();
        }
      } else if(position > m_last_position &&
          scroll_bar.get_range().m_end - position <
            scroll_bar.get_page_size() / 2) {
        auto table =
          std::static_pointer_cast<TimeAndSalesTableModel>(
            m_table_view->get_table());
        table->load_history(scroll_box.height() /
          m_table_view->get_body().estimate_scroll_line_height());
      }
      m_last_position = position;
    }

    void on_begin_loading() {
      if(m_is_loading) {
        return;
      }
      m_is_loading = true;
      const auto PULL_DELAY_TIMEOUT_MS = 1000;
      m_timer.start(PULL_DELAY_TIMEOUT_MS);
    }

    void on_end_loading() {
      m_is_loading = false;
      m_spinner->stop();
      m_timer.stop();
      hide();
    }

    void on_timeout() {
      if(!m_is_loading) {
        return;
      }
      m_spinner->start();
      display();
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
      update_style(table_view.get_header(), [&] (auto& style) {
        style.get(Any()).
          set(PaddingLeft(scale_width(4))).
          set(PaddingTop(scale_height(5))).
          set(PaddingBottom(scale_height(4)));
      });
      update_style(table_view.get_body(), [&] (auto& style) {
        style.get(Any()).
          set(grid_color(QColor(0xE0E0E0))).
          set(padding(0)).
          set(spacing(0)).
          set(PaddingBottom(TABLE_BODY_BOTTOM_PADDING()));
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
      on_properties(m_properties->get());
      m_connection = m_properties->connect_update_signal(
        std::bind_front(&TableViewStylist::on_properties, this));
    }

    void on_properties(const TimeAndSalesProperties& properties) {
      const auto DEBOUNCE_TIME_MS = 100;
      QTimer::singleShot(DEBOUNCE_TIME_MS, [=] {
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
            style.get(Any() > +Row() > is_a<TableItem>() > selector).
              set(BackgroundColor(highlight.m_background_color));
          }
        });
        if(properties.is_grid_enabled()) {
          match(table_view, ShowGrid());
        } else {
          unmatch(table_view, ShowGrid());
        }
        auto& header = table_view.get_header();
        for(auto i = 0; i != TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
          if(properties.is_visible(
              static_cast<TimeAndSalesTableModel::Column>(i))) {
            table_view.show_column(i);
          } else {
            table_view.hide_column(i);
          }
        }
      });
    }
  };
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table,
    std::shared_ptr<TimeAndSalesPropertiesModel> properties, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(RecycledTableViewItemBuilder(ItemBuilder())).
    set_current(
      std::make_shared<ConstantValueModel<optional<TableIndex>>>(none)).
    make();
  make_header_menu(*table_view, properties);
  auto pull_indicator = new PullIndicator(*table_view);
  auto stylist = new TableViewStylist(*table_view, std::move(properties));
  return table_view;
}
