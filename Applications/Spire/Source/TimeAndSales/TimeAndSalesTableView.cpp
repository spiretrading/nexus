#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QMouseEvent>
#include <QMovie>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/SaleConditionBox.hpp"
#include "Spire/Ui/SaleConditionListItem.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using TimeAndSalesBody = StateSelector<void, struct TimeAndSalesBodyTag>;

  auto TABLE_VIEW_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    style.get(Any() > is_a<TableBody>()).
      set(grid_color(Qt::transparent)).
      set(horizontal_padding(0)).
      set(vertical_padding(0)).
      set(HorizontalSpacing(0)).
      set(VerticalSpacing(0));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > PullIndicator()).set(Visibility::NONE);
    style.get(Any() > CurrentRow()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > TableHeaderItem::Label()).
      set(TextStyle(font, QColor(0x595959)));
    style.get(NoAdditionalEntries() > TimeAndSalesBody()).
      set(PaddingBottom(scale_height(44)));
    style.get(PullDelayed() > TimeAndSalesBody()).
      set(PaddingBottom(0));
    style.get(PullDelayed() > PullIndicator()).
      set(Visibility::VISIBLE);
    return style;
  }

  auto TABLE_HADER_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
    return style;
  }

  auto TABLE_CELL_STYLE(StyleSheet style) {
    style.get(Any()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(1.5)));
    return style;
  }

  auto PULL_INDICATOR_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BodyAlign(Qt::AlignHCenter)).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(20))).
      set(PaddingTop(scale_height(8)));
    return style;
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

  auto make_pull_indicator() {
    auto spinner = new QMovie(":/Icons/spinner.gif", QByteArray());
    spinner->setScaledSize(scale(16, 16));
    spinner->start();
    auto spinner_widget = new QLabel();
    spinner_widget->setMovie(spinner);
    auto box = new Box(spinner_widget);
    set_style(*box, PULL_INDICATOR_STYLE());
    return box;
  }

  auto make_time_cell(const ptime& time) {
    auto ts = displayText(time);
    ts = ts.left(ts.lastIndexOf('.'));
    auto time_box = make_time_box(duration_from_string(ts.toStdString()));
    time_box->set_read_only(true);
    update_style(*time_box, [] (auto& style) {
      style.get(ReadOnly() > is_a<TextBox>()).
        set(border_size(0)).
        set(horizontal_padding(0)).
        set(vertical_padding(0));
      style.get(Any() > is_a<DecimalBox>()).
        set(TrailingZeros(0));
      style.get(Any()).
        set(PaddingLeft(scale_width(2))).
        set(PaddingRight(0));
    });
    auto& box = *static_cast<Box*>(time_box->layout()->itemAt(0)->widget());
    auto box_body = box.get_body();
    for(auto i = 0; i < box_body->layout()->count(); ++i) {
      auto field = box_body->layout()->itemAt(i)->widget();
      field->setMinimumWidth(0);
      field->setMaximumWidth(QWIDGETSIZE_MAX);
      static_cast<QHBoxLayout*>(box_body->layout())->setStretchFactor(field, 0);
    }
    time_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return time_box;
  }

  template<typename B, typename T =
    std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar>
  auto make_decimal_cell(const T& value) {
    auto cell = new B();
    cell->get_current()->set(value);
    cell->set_read_only(true);
    update_style(*cell, [] (auto& style) {
      style.get(Any() > is_a<TextBox>()).
        set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
    });
    return cell;
  }

  auto make_market_cell(const std::string& market_code) {
    auto query_model = std::make_shared<LocalComboBoxQueryModel>();
    auto market = GetDefaultMarketDatabase().FromCode(market_code);
    query_model->add(displayText(MarketToken(market.m_code)).toLower(), market);
    query_model->add(QString(market.m_code.GetData()).toLower(), market);
    auto market_box = new MarketBox(std::move(query_model),
      std::make_shared<LocalValueModel<MarketCode>>(market_code));
    market_box->set_read_only(true);
    return market_box;
  }

  auto make_condition_cell(const TimeAndSale::Condition& condition) {
    auto query_model = std::make_shared<LocalComboBoxQueryModel>();
    query_model->add(QString::fromStdString(condition.m_code).toLower(),
      SaleConditionInfo(condition, ""));
    auto condition_box = new SaleConditionBox(std::move(query_model),
      std::make_shared<LocalValueModel<TimeAndSale::Condition>>(condition));
    condition_box->set_read_only(true);
    return condition_box;
  }
}

struct TimeAndSalesTableView::TimeAndSalesTableViewModel : public TableModel {
  std::shared_ptr<TimeAndSalesTableModel> m_source;

  explicit TimeAndSalesTableViewModel(
    std::shared_ptr<TimeAndSalesTableModel> source)
    : m_source(std::move(source)) {}

  int get_row_size() const override {
    return m_source->get_row_size();
  }

  int get_column_size() const override {
    return m_source->get_column_size() + 1;
  }

  AnyRef at(int row, int column) const override {
    if(column == m_source->get_column_size()) {
      return {};
    }
    return m_source->at(row, column);
  }

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override {
    return m_source->connect_operation_signal(slot);
  }
};

TimeAndSalesTableView::TimeAndSalesTableView(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent)
    : m_table(std::make_shared<TimeAndSalesTableViewModel>(std::move(table))),
      m_timer(new QTimer(this)),
      m_is_loading(false),
      m_has_additional_entries(false),
      m_last_scroll_y(0),
      m_resize_index(-1),
      m_begin_loading_connection(
        m_table->m_source->connect_begin_loading_signal(
          std::bind_front(&TimeAndSalesTableView::on_begin_loading, this))),
      m_end_loading_connection(m_table->m_source->connect_end_loading_signal(
        std::bind_front(&TimeAndSalesTableView::on_end_loading, this))) {
  make_header_item_properties();
  m_table_view = TableViewBuilder(m_table).
    set_header(make_header_model()).
    set_view_builder(
      std::bind_front(&TimeAndSalesTableView::table_view_builder, this)).
    make();
  m_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_table_view, [] (auto& style) {
    style = TABLE_VIEW_STYLE(style);
  });
  enclose(*this, *m_table_view);
  customize_table_header();
  customize_table_body();
  make_table_columns_sub_menu();
  setFocusProxy(m_table_view);
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout,
    std::bind_front(&TimeAndSalesTableView::on_timer_expired, this));
  m_table_operation_connection = m_table->connect_operation_signal(
    std::bind_front(&TimeAndSalesTableView::on_table_operation, this));
}

const std::shared_ptr<TimeAndSalesTableModel>&
    TimeAndSalesTableView::get_table() const {
  return m_table->m_source;
}

const TableItem* TimeAndSalesTableView::get_item(Index index) const {
  return m_table_body->get_item(index);
}

TableItem* TimeAndSalesTableView::get_item(Index index) {
  return m_table_body->get_item(index);
}

bool TimeAndSalesTableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table_header) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::RightButton) {
        m_table_columns_menu->window()->move(mouse_event.globalPos());
        m_table_columns_menu->show();
      }
    } else if(event->type() == QEvent::MouseMove) {
      if(m_resize_index >= 0) {
        resize_column_widths();
        return true;
      }
    }
  } else if(watched == m_table_body && event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Up || key_event.key() == Qt::Key_Down) {
      if(m_table->get_row_size() > 0) {
        m_body_scroll_box->get_vertical_scroll_bar().set_line_size(
          get_item({0, 0})->height());
      }
    }
    event->ignore();
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

QWidget* TimeAndSalesTableView::table_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto column_id = static_cast<Column>(column);
  auto cell = [&] () -> QWidget* {
    if(column_id == Column::TIME) {
      return make_time_cell(table->get<ptime>(row, column));
    } else if(column_id == Column::PRICE) {
      return make_decimal_cell<MoneyBox>(table->get<Money>(row, column));
    } else if(column_id == Column::SIZE) {
      return make_decimal_cell<QuantityBox>(table->get<Quantity>(row, column));
    } else if(column_id == Column::MARKET) {
      return make_market_cell(table->get<std::string>(row, column));
    } else if(column_id == Column::CONDITION) {
      return
        make_condition_cell(table->get<TimeAndSale::Condition>(row, column));
    }
    return make_label("");
  }();
  if(column_id != Column::TIME) {
    update_style(*cell, [] (auto& style) {
      style = TABLE_CELL_STYLE(style);
    });
  }
  return cell;
}

int TimeAndSalesTableView::get_next_sibling_index(int index) const {
  if(index == m_table_header->get_widths()->get_size() - 1) {
    return index;
  }
  ++index;
  if(!m_header_item_properties[index].m_is_visible) {
    return get_next_sibling_index(index);
  }
  return index;
}

void TimeAndSalesTableView::make_header_item_properties() {
  m_header_item_properties.emplace_back(false, Qt::AlignLeft, scale_width(45));
  m_header_item_properties.emplace_back(true, Qt::AlignRight, scale_width(50));
  m_header_item_properties.emplace_back(true, Qt::AlignRight, scale_width(40));
  m_header_item_properties.emplace_back(true, Qt::AlignLeft, scale_width(38));
  m_header_item_properties.emplace_back(false, Qt::AlignLeft, scale_width(34));
  m_header_item_properties.emplace_back(true, Qt::AlignLeft, 0);
}

void TimeAndSalesTableView::make_table_columns_sub_menu() {
  m_table_columns_menu = new ContextMenu(*this);
  auto add_sub_menu = [&] (int column, const QString& name, bool checked) {
    auto model = m_table_columns_menu->add_check_box(name);
    model->set(checked);
    model->connect_update_signal(std::bind_front(
      &TimeAndSalesTableView::on_column_sub_menu_check, this, column));
  };
  for(auto i = 0; i < TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
    add_sub_menu(i, m_table_header->get_items()->get(i).m_name,
      m_header_item_properties[i].m_is_visible);
  }
}

void TimeAndSalesTableView::customize_table_header() {
  auto& header_box =
    *static_cast<Box*>(m_table_view->layout()->itemAt(0)->widget());
  m_table_header = static_cast<TableHeader*>(header_box.get_body()->layout()->
      itemAt(0)->widget());
  m_table_header->installEventFilter(this);
  m_header_scroll_box = new ScrollBox(m_table_header);
  m_header_scroll_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Fixed);
  m_header_scroll_box->setFocusPolicy(Qt::NoFocus);
  m_header_scroll_box->set_horizontal(ScrollBox::DisplayPolicy::NEVER);
  m_header_scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
  update_style(*m_header_scroll_box, [] (auto& style) {
    style = TABLE_HADER_STYLE(style);
  });
  auto old_header_box =
    m_table_view->layout()->replaceWidget(&header_box, m_header_scroll_box);
  delete old_header_box->widget();
  delete old_header_box;
  auto layout = m_table_header->layout();
  for(auto i = 0; i < TimeAndSalesTableModel::COLUMN_SIZE; ++i) {
    auto& item = *static_cast<TableHeaderItem*>(
      m_table_header->layout()->itemAt(i)->widget());
    auto item_layout = item.layout();
    item_layout->setContentsMargins(
      {scale_width(4), scale_height(5), 0, scale_height(2)});
    auto contents_layout =
      item_layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
    contents_layout->setContentsMargins({});
    auto labels = std::make_shared<ArrayListModel<QString>>();
    labels->push(m_table_header->get_items()->get(i).m_name);
    labels->push(m_table_header->get_items()->get(i).m_short_name);
    auto name_label = new ResponsiveLabel(labels);
    auto old_label = contents_layout->replaceWidget(
      contents_layout->itemAt(0)->widget(), name_label);
    delete old_label->widget();
    delete old_label;
    match(*name_label, TableHeaderItem::Label());
    if(m_header_item_properties[i].m_text_align == Qt::AlignRight) {
      static_cast<QSpacerItem*>(contents_layout->itemAt(1))->changeSize(0, 0);
      contents_layout->itemAt(2)->widget()->setFixedWidth(0);
      contents_layout->itemAt(3)->widget()->setFixedWidth(0);
      update_style(item, [] (auto& style) {
        style.get(Any() > TableHeaderItem::Label()).
          set(TextAlign(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter)));
      });
    }
    if(m_header_item_properties[i].m_is_visible) {
      m_table_header->get_widths()->set(i, m_header_item_properties[i].m_width);
    } else {
      m_table_header->get_widths()->set(i, 0);
    }
    item.connect_start_resize_signal(
      std::bind_front(&TimeAndSalesTableView::on_start_resize, this, i));
    item.connect_end_resize_signal(
      std::bind_front(&TimeAndSalesTableView::on_end_resize, this, i));
  }
  customize_empty_header_cell();
}

void TimeAndSalesTableView::customize_empty_header_cell() {
  auto& empty_header_cell = *static_cast<TableHeaderItem*>(
    m_table_header->layout()->itemAt(
      TimeAndSalesTableModel::COLUMN_SIZE)->widget());
  auto layout = empty_header_cell.layout();
  layout->setContentsMargins(
    {m_header_scroll_box->get_vertical_scroll_bar().sizeHint().width(),
      scale_height(5), 0, 0});
  auto contents_layout =
    layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
  contents_layout->setContentsMargins({});
  contents_layout->itemAt(2)->widget()->setFixedWidth(0);
  contents_layout->itemAt(3)->widget()->setFixedWidth(0);
  auto bottom_layout = layout->itemAt(1)->layout();
  bottom_layout->setContentsMargins({0, scale_height(2), 0, 0});
  bottom_layout->itemAt(0)->widget()->setFixedWidth(0);
}

void TimeAndSalesTableView::customize_table_body() {
  auto& old_scroll_box =
    *static_cast<ScrollBox*>(m_table_view->layout()->itemAt(1)->widget());
  m_table_body = static_cast<TableBody*>(&old_scroll_box.get_body());
  m_table_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_table_body->installEventFilter(this);
  auto time_and_sales_body = new Box(m_table_body);
  match(*time_and_sales_body, TimeAndSalesBody());
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(time_and_sales_body);
  auto pull_indicator = make_pull_indicator();
  pull_indicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  match(*pull_indicator, PullIndicator());
  body_layout->addWidget(pull_indicator);
  body_layout->addStretch(1);
  m_body_scroll_box = new ScrollBox(body);
  m_body_scroll_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_body_scroll_box->setFocusPolicy(Qt::NoFocus);
  m_body_scroll_box->get_horizontal_scroll_bar().connect_position_signal(
    std::bind_front(&TimeAndSalesTableView::on_horizontal_scroll_position,
      this));
  m_body_scroll_box->get_vertical_scroll_bar().connect_position_signal(
    std::bind_front(&TimeAndSalesTableView::on_vertical_scroll_position, this));
  auto layout_item =
    m_table_view->layout()->replaceWidget(&old_scroll_box, m_body_scroll_box);
  delete layout_item->widget();
  delete layout_item;
  auto stretch_item = m_table_view->layout()->itemAt(2);
  m_table_view->layout()->removeItem(stretch_item);
  delete stretch_item;
  m_table_view->setFocusProxy(m_table_body);
}

void TimeAndSalesTableView::resize_column_widths() {
  auto position = QCursor::pos();
  auto delta = position.x() - m_resize_position.x();
  if(delta < 0) {
    auto width = m_table_header->get_widths()->get(m_resize_index);
    auto new_width = std::max(scale_width(10), width + delta);
    delta = new_width - width;
    position.rx() = delta + m_resize_position.x();
    if(delta != 0) {
      m_table_header->get_widths()->set(m_resize_index,
        m_table_header->get_widths()->get(m_resize_index) + delta);
      auto sibling_index = get_next_sibling_index(m_resize_index);
      if(sibling_index != m_table_header->get_widths()->get_size() - 1) {
        m_table_header->get_widths()->set(sibling_index,
          m_table_header->get_widths()->get(sibling_index) - delta);
      }
    }
  } else if(delta > 0) {
    auto sibling_index = get_next_sibling_index(m_resize_index);
    auto sibling_width = [&] {
      if(sibling_index == m_table_header->get_widths()->get_size() - 1) {
        auto w = 0;
        for(auto i = 0; i < m_table_header->get_widths()->get_size(); ++i) {
          w += m_table_header->get_widths()->get(i);
        }
        return m_table_header->width() - w;
      }
      return m_table_header->get_widths()->get(sibling_index);
    }();
    auto new_sibling_width = std::max(scale_width(10), sibling_width - delta);
    delta = new_sibling_width - sibling_width;
    position.rx() = -delta + m_resize_position.x();
    if(delta != 0) {
      if(sibling_index != m_table_header->get_widths()->get_size() - 1) {
        m_table_header->get_widths()->set(sibling_index,
          m_table_header->get_widths()->get(sibling_index) + delta);
      }
      m_table_header->get_widths()->set(m_resize_index,
        m_table_header->get_widths()->get(m_resize_index) - delta);
    }
  }
  m_resize_position = position;
}

void TimeAndSalesTableView::on_column_sub_menu_check(int column, bool checked) {
  auto width = [&] {
    if(checked) {
      return m_header_item_properties[column].m_width;
    }
    return 0;
  }();
  m_table_header->get_widths()->set(column, width);
  m_header_item_properties[column].m_is_visible = checked;
}

void TimeAndSalesTableView::on_begin_loading() {
  if(m_is_loading) {
    return;
  }
  m_is_loading = true;
  m_has_additional_entries = false;
  m_timer->start(1000);
}

void TimeAndSalesTableView::on_end_loading() {
  m_is_loading = false;
  if(m_has_additional_entries) {
    unmatch(*m_table_view, NoAdditionalEntries());
  } else {
    match(*m_table_view, NoAdditionalEntries());
  }
  m_timer->stop();
  unmatch(*m_table_view, PullDelayed());
}

void TimeAndSalesTableView::on_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_has_additional_entries = true;
      get_item({operation.m_index, 0})->parentWidget()->show();
      m_body_scroll_box->get_body().adjustSize();
    });
}

void TimeAndSalesTableView::on_horizontal_scroll_position(int position) {
  m_header_scroll_box->get_horizontal_scroll_bar().set_position(position);
}

void TimeAndSalesTableView::on_vertical_scroll_position(int position) {
  if(!m_is_loading && position > m_last_scroll_y) {
    auto& scroll_bar = m_body_scroll_box->get_vertical_scroll_bar();
    if(scroll_bar.get_range().m_end - position <
        scroll_bar.get_page_size() / 2) {
      m_table->m_source->load_history(
        m_body_scroll_box->height() / get_item(Index{0,0})->height());
    }
  }
  m_last_scroll_y = position;
}

void TimeAndSalesTableView::on_timer_expired() {
  match(*m_table_view, PullDelayed());
  m_body_scroll_box->get_body().adjustSize();
  scroll_to_end(m_body_scroll_box->get_vertical_scroll_bar());
}

void TimeAndSalesTableView::on_start_resize(int index) {
  m_resize_index = index;
  m_resize_position = QCursor::pos();
}

void TimeAndSalesTableView::on_end_resize(int index) {
  m_resize_index = -1;
}
