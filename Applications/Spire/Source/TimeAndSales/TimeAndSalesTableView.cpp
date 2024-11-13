#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QMovie>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
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

  auto TABLE_BODY_BOTTOM_PADDING() {
    static auto height = scale_height(44);
    return height;
  }

  struct HeaderItemProperties {
    Qt::Alignment m_alignment;
    int m_width;
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
      set(padding(0)).
      set(spacing(0));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > CurrentRow()).
      set(BackgroundColor(Qt::transparent));
    style.get(body_selector > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    style.get(Any() > is_a<TableBody>()).
      set(PaddingBottom(TABLE_BODY_BOTTOM_PADDING()));
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
    add_item(QObject::tr("Buyer"), QObject::tr("Buy"));
    add_item(QObject::tr("Seller"), QObject::tr("Sell"));
    add_item("", "");
    return model;
  }

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

  void initialize_table_header(
      TableView& table_view, const TimeAndSalesProperties& properties) {
    auto header_properties = make_header_item_properties();
    auto& header = table_view.get_header();
    for(auto i = 0; i != TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
      if(properties.is_visible(
          static_cast<TimeAndSalesTableModel::Column>(i))) {
        table_view.show_column(i);
      } else {
        table_view.hide_column(i);
      }
    }
    for(auto i = 0; i < std::ssize(header_properties); ++i) {
      header.get_widths()->set(i, header_properties[i].m_width);
      update_style(*header.get_item(i), [&] (auto& style) {
        style.get(Any() > TableHeaderItem::Label()).
          set(TextAlign(header_properties[i].m_alignment));
      });
    }
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
      } else if(column_id == TimeAndSalesTableModel::Column::BUYER) {
        return make_label(to_text(table->get<std::string>(row, column)));
      } else if(column_id == TimeAndSalesTableModel::Column::SELLER) {
        return make_label(to_text(table->get<std::string>(row, column)));
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
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table,
    std::shared_ptr<TimeAndSalesPropertiesModel> properties, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(std::bind_front(&item_builder, table)).make();
  update_style(*table_view, apply_table_view_style);
  initialize_table_header(*table_view, properties->get());
  make_header_menu(*table_view, properties);
  auto pull_indicator = new PullIndicator(*table_view);
  return table_view;
}
