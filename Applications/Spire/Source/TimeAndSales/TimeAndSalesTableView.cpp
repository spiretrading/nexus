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

  auto make_header_menu(TableView& table_view,
      const std::vector<HeaderItemProperties>& properties) {
    auto header = &table_view.get_header();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    auto menu = new ContextMenu(*header);
    for(auto i = 0; i < std::ssize(properties); ++i) {
      auto model = menu->add_check_box(header->get_items()->get(i).m_name);
      model->connect_update_signal(
        [i, table_view_ptr = &table_view] (auto checked) {
          if(checked) {
            table_view_ptr->show_column(i);
          } else {
            table_view_ptr->hide_column(i);
          }
        });
      model->set(properties[i].m_is_visible);
    }
    QObject::connect(header, &QWidget::customContextMenuRequested,
      [=] (const auto& pos) {
        menu->window()->move(header->mapToGlobal(pos));
        menu->window()->show();
      });
  }

  auto make_pull_indicator(QWidget* parent) {
    auto spinner = new QMovie(":/Icons/spinner.gif");
    spinner->setScaledSize(scale(16, 16));
    spinner->start();
    auto spinner_widget = new QLabel();
    spinner_widget->setMovie(spinner);
    auto indicator_box = new Box(spinner_widget, parent);
    indicator_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*indicator_box, [] (auto& style) {
      style.get(Any()).
        set(Visibility::NONE).
        set(BodyAlign(Qt::AlignHCenter)).
        set(horizontal_padding(scale_width(8))).
        set(PaddingBottom(scale_height(20))).
        set(PaddingTop(scale_height(8)));
    });
    return indicator_box;
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

  struct PullIndicator : QWidget {
    PullIndicator(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto spinner = new QMovie(":/Icons/spinner.gif");
      spinner->setScaledSize(scale(16, 16));
      spinner->start();
      auto spinner_widget = new QLabel();
      spinner_widget->setMovie(spinner);
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
      hide();
      parent->installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Resize && isVisible()) {
        auto& resize_event = *static_cast<QResizeEvent*>(event);
        update_position(resize_event.size());
      }
      return QObject::eventFilter(watched, event);
    }

    void update_position(const QSize& size) {
      setGeometry(0, size.height() - TABLE_BODY_BOTTOM_PADDING(),
        size.width(), TABLE_BODY_BOTTOM_PADDING());
    }
  };
}

TableView* Spire::make_time_and_sales_table_view(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(std::bind_front(&item_builder, table)).make();
  update_style(*table_view, apply_table_view_style);
  auto properties = make_header_item_properties();
  make_header_menu(*table_view, properties);
  auto& header = table_view->get_header();
  for(auto i = 0; i < std::ssize(properties); ++i) {
    header.get_widths()->set(i, properties[i].m_width);
    update_style(*header.get_item(i), [&] (auto& style) {
      style.get(Any() > TableHeaderItem::Label()).
        set(TextAlign(properties[i].m_alignment));
    });
  }
  auto body = &table_view->get_body();
  auto pull_indicator = new PullIndicator(body);
  auto status = std::make_shared<Status>(false, 0);
  auto scroll_box = &table_view->get_scroll_box();
  scroll_box->get_vertical_scroll_bar().connect_position_signal(
    [=] (int position) {
      auto& scroll_bar = scroll_box->get_vertical_scroll_bar();
      if(!status->m_is_loading && position > status->m_last_scroll_y &&
          scroll_bar.get_range().m_end - position <
            scroll_bar.get_page_size() / 2) {
        table->load_history(
          scroll_box->height() / body->estimate_scroll_line_height());
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
      pull_indicator->update_position(body->size());
      pull_indicator->show();
    });
  });
  table->connect_end_loading_signal([=] {
    status->m_is_loading = false;
    pull_indicator->hide();
  });
  return table_view;
}
