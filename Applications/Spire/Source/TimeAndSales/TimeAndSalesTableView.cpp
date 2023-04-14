#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QMovie>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/Icon.hpp"
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
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto TABLE_VIEW_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF)));
    style.get(Any() > is_a<TableBody>()).
      set(horizontal_padding(0)).
      set(HorizontalSpacing(0)).
      set(PaddingBottom(0)).
      set(VerticalSpacing(0));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > is_a<TableHeaderItem>() > TableHeaderItem::Label()).
      set(TextStyle(font, QColor(0x595959)));
    //style.get(Any() > is_a<PullIndicator>()).
    //  set(Visibility::INVISIBLE);
    //style.get(Any() > PullDelayed() > is_a<PullIndicator>()).
    //  set(Visibility::VISIBLE);
    return style;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({QObject::tr("Time"), QObject::tr("Time"),
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({QObject::tr("Price"), QObject::tr("Px"),
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({QObject::tr("Size"), QObject::tr("Sz"),
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({QObject::tr("Market"), QObject::tr("Mkt"),
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({QObject::tr("Condition"), QObject::tr("Cond"),
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

  auto make_pull_indicator() {
    auto spinner = new QMovie(":/Icons/spinner.gif", QByteArray());
    spinner->setScaledSize(scale(16, 16));
    spinner->start();
    auto spinner_widget = new QLabel();
    spinner_widget->setMovie(spinner);
    auto box = new Box(spinner_widget);
    auto style = StyleSheet();
    style.get(Any()).
      set(BodyAlign(Qt::AlignHCenter)).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(20))).
      set(PaddingTop(scale_height(8)));
    set_style(*box, std::move(style));
    return box;
  }
}

TimeAndSalesTableView::TimeAndSalesTableView(
    std::shared_ptr<TimeAndSalesTableModel> table, QWidget* parent)
    : m_table(std::move(table)),
      m_timer(new QTimer(this)),
      m_is_loading(false),
      m_begin_loading_connection(m_table->connect_begin_loading_signal(
        std::bind_front(&TimeAndSalesTableView::on_begin_loading, this))),
      m_end_loading_connection(m_table->connect_end_loading_signal(
        std::bind_front(&TimeAndSalesTableView::on_end_loading, this))) {
  auto current = std::make_shared<LocalValueModel<optional<Index>>>();
  current->connect_update_signal([=] (const auto& value) {
    if(value) {
      current->set(none);
    }
  });
  auto table_view = TableViewBuilder(m_table).
    set_header(make_header_model()).
    set_current(current).
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
  customize_table_header();
  align_header_item_right(Column::PRICE);
  align_header_item_right(Column::SIZE);
  auto& old_scroll_box =
    *static_cast<ScrollBox*>(table_view->layout()->itemAt(1)->widget());
  m_table_body = static_cast<TableBody*>(&old_scroll_box.get_body());
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(m_table_body);
  m_pull_indicator = make_pull_indicator();
  m_pull_indicator->setVisible(false);
  body_layout->addWidget(m_pull_indicator);
  m_scroll_box = new ScrollBox(body);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  auto layout_item = table_view->layout()->replaceWidget(&old_scroll_box, m_scroll_box);
  delete layout_item->widget();
  delete layout_item;
  m_scroll_box->get_vertical_scroll_bar().connect_position_signal(
    std::bind_front(&TimeAndSalesTableView::on_scroll_position, this));
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout,
    std::bind_front(&TimeAndSalesTableView::on_timer_expired, this));
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
  auto column_id = static_cast<Column>(column);
  if(column_id == Column::TIME) {
    auto time_box = make_time_box(table->get<ptime>(row, column).time_of_day());
    time_box->set_read_only(true);
    time_box->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    return time_box;
  } else if(column_id == Column::PRICE) {
    auto modifiers = QHash<Qt::KeyboardModifier, Money>(
      {{Qt::NoModifier, Money::ONE}, {Qt::AltModifier, 5 * Money::ONE},
      {Qt::ControlModifier, 10 * Money::ONE}, {Qt::ShiftModifier, 20 * Money::ONE}});
    auto money_box = new MoneyBox(std::make_shared<LocalOptionalMoneyModel>(table->get<Money>(row, column)),
      std::move(modifiers));
    money_box->set_read_only(true);
    update_style(*money_box, [] (auto& style) {
      style.get(Any() > is_a<TextBox>()).
        set(TextAlign(Qt::AlignRight));
    });
    return money_box;
  } else if(column_id == Column::SIZE) {
    auto modifiers = QHash<Qt::KeyboardModifier, Quantity>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
        {Qt::ShiftModifier, 20}});
    auto quantity_box = new QuantityBox(std::make_shared<LocalOptionalQuantityModel>(table->get<Quantity>(row, column)),
      std::move(modifiers));
    quantity_box->set_read_only(true);
    update_style(*quantity_box, [] (auto& style) {
      style.get(Any() > is_a<TextBox>()).
        set(TextAlign(Qt::AlignRight));
    });
    return quantity_box;
  } else if(column_id == Column::MARKET) {
      auto market_code = table->get<std::string>(row, column);
      auto query_model = std::make_shared<LocalComboBoxQueryModel>();
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      query_model->add(displayText(MarketToken(market.m_code)).toLower(), market);
      query_model->add(QString(market.m_code.GetData()).toLower(), market);
      auto market_box = new MarketBox(std::move(query_model),
        std::make_shared<LocalValueModel<MarketCode>>(market_code));
      market_box->set_read_only(true);
      return market_box;
  } else if(column_id == Column::CONDITION) {
    auto condition = table->get<TimeAndSale::Condition>(row, column);
    auto condition_info = SaleConditionInfo(condition, "");
    auto query_model = std::make_shared<LocalComboBoxQueryModel>();
    query_model->add(QString::fromStdString(condition.m_code).toLower(),
      condition_info);
    auto condition_box = new SaleConditionBox(std::move(query_model),
      std::make_shared<LocalValueModel<TimeAndSale::Condition>>(condition));
    condition_box->set_read_only(true);
    return condition_box;
  }
  return nullptr;
}

void TimeAndSalesTableView::align_header_item_right(Column column) {
  auto header_item =
    m_table_header->layout()->itemAt(static_cast<int>(column))->widget();
  auto contents_layout =
    header_item->layout()->itemAt(0)->layout()->itemAt(0)->widget()->layout();
  static_cast<QSpacerItem*>(contents_layout->itemAt(1))->changeSize(0, 0);
  contents_layout->itemAt(2)->widget()->setFixedWidth(0);
  contents_layout->itemAt(3)->widget()->setFixedWidth(0);
  update_style(*header_item, [] (auto& style) {
    style.get(Any() > TableHeaderItem::Label()).
      set(TextAlign(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter)));
  });
}

void TimeAndSalesTableView::customize_table_header() {
  auto layout = m_table_header->layout();
  for(auto i = 0; i < layout->count(); ++i) {
    auto header_item = m_table_header->layout()->itemAt(i)->widget();
    auto header_item_layout = header_item->layout();
    header_item_layout->setContentsMargins({0, scale_height(5), 0, scale_height(2)});
    auto contents_layout =
      header_item_layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
    contents_layout->setContentsMargins({scale_width(4), 0, 0, 0});
    auto labels = std::make_shared<ArrayListModel<QString>>();
    labels->push(m_table_header->get_items()->get(i).m_name);
    labels->push(m_table_header->get_items()->get(i).m_short_name);
    auto name_label = new ResponsiveLabel(labels);
    auto old = contents_layout->replaceWidget(contents_layout->itemAt(0)->widget(), name_label);
    delete old->widget();
    delete old;
    match(*name_label, TableHeaderItem::Label());
  }
}

void TimeAndSalesTableView::on_begin_loading() {
  if(m_is_loading) {
    return;
  }
  m_is_loading = true;
  m_timer->start(1000);
}

void TimeAndSalesTableView::on_end_loading() {
  m_is_loading = false;
  m_timer->stop();
  m_pull_indicator->setVisible(false);
}

void TimeAndSalesTableView::on_scroll_position(int position) {
  if(m_is_loading) {
    return;
  }
  auto& scroll_bar = m_scroll_box->get_vertical_scroll_bar();
  if(m_scroll_box->get_body().height() - position - m_scroll_box->get_vertical_scroll_bar().get_page_size() <
      m_scroll_box->get_vertical_scroll_bar().get_page_size() / 2) {
    m_table->load_history(10);
  }
}

void TimeAndSalesTableView::on_timer_expired() {
  m_pull_indicator->setVisible(true);
  m_scroll_box->get_body().adjustSize();
  scroll_to_end(m_scroll_box->get_vertical_scroll_bar());
}
