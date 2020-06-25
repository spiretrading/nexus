#include "Spire/BookView/BookViewHighlightPropertiesWidget.hpp"
#include <QButtonGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QShowEvent>
#include <QVBoxLayout>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/BookView/MarketListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewHighlightPropertiesWidget::BookViewHighlightPropertiesWidget(
    const BookViewProperties& properties, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, scale_height(8), 0, scale_height(20));
  layout->setSpacing(0);
  auto markets_layout = new QVBoxLayout();
  markets_layout->setContentsMargins({});
  markets_layout->setSpacing(0);
  auto markets_label = new QLabel(tr("Markets"), this);
  auto generic_header_label_stylesheet = QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(12));
  markets_label->setStyleSheet(generic_header_label_stylesheet);
  markets_layout->addWidget(markets_label, 14);
  markets_layout->addStretch(10);
  auto markets_scroll_area = new ScrollArea(this);
  markets_scroll_area->setFixedWidth(scale_width(140));
  markets_scroll_area->set_border_style(scale_width(1), QColor("#C8C8C8"));
  markets_scroll_area->setObjectName("markets_scroll_area");
  markets_scroll_area->setWidgetResizable(true);
  markets_layout->addWidget(markets_scroll_area, 222);
  m_markets_list_widget = new QListWidget(this);
  markets_scroll_area->setWidget(m_markets_list_widget);
  m_markets_list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  for(auto& entry : GetDefaultMarketDatabase().GetEntries()) {
    auto item = new MarketListItem(entry, m_markets_list_widget);
    item->setTextAlignment(Qt::AlignCenter);
    auto highlight = properties.get_market_highlight(entry.m_code);
    if(highlight.is_initialized()) {
      item->setBackground(highlight->m_color);
      item->set_highlight_color(highlight->m_color);
      if(highlight->m_highlight_all_levels) {
        item->set_highlight_all_levels();
      } else {
        item->set_highlight_top_level();
      }
    } else {
      item->setBackground(Qt::white);
    }
  }
  m_markets_list_widget->item(0)->setSelected(true);
  m_markets_list_widget->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  m_markets_list_widget->setSelectionBehavior(
    QAbstractItemView::SelectionBehavior::SelectRows);
  connect(m_markets_list_widget, &QListWidget::currentRowChanged,
    [=] { update_market_widgets(); });
  layout->addLayout(markets_layout, 140);
  layout->addStretch(18);
  auto market_highlight_layout = new QVBoxLayout();
  market_highlight_layout->setContentsMargins({});
  market_highlight_layout->setSpacing(0);
  market_highlight_layout->addStretch(26);
  m_highlight_none_check_box = new CheckBox(tr("Highlight None"), this);
  auto check_box_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;
    outline: none;
    spacing: %2px;)")
    .arg(scale_height(12)).arg(scale_width(4));
  auto check_box_indicator_style = QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    height: %3px;
    width: %4px;)").arg(scale_height(1))
    .arg(scale_width(1)).arg(scale_height(15)).arg(scale_width(15));
  auto check_box_checked_style = QString(R"(
    image: url(:/Icons/check-with-box.svg);)");
  auto check_box_hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto check_box_focused_style = QString(R"(border-color: #4B23A0;)");
  m_highlight_none_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  connect(m_highlight_none_check_box, &QCheckBox::stateChanged, this,
    &BookViewHighlightPropertiesWidget::
    on_highlight_none_check_box_checked);
  market_highlight_layout->addWidget(m_highlight_none_check_box, 16);
  market_highlight_layout->addStretch(10);
  m_highlight_top_level_check_box = new CheckBox(tr("Highlight Top Level"),
    this);
  m_highlight_top_level_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  connect(m_highlight_top_level_check_box, &QCheckBox::stateChanged, this,
    &BookViewHighlightPropertiesWidget::
    on_highlight_top_level_check_box_checked);
  market_highlight_layout->addWidget(m_highlight_top_level_check_box, 16);
  market_highlight_layout->addStretch(10);
  m_highlight_all_levels_check_box = new CheckBox(
    tr("Highlight All Levels"), this);
  m_highlight_all_levels_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  connect(m_highlight_all_levels_check_box, &QCheckBox::stateChanged, this,
    &BookViewHighlightPropertiesWidget::
    on_highlight_all_levels_check_box_checked);
  market_highlight_layout->addWidget(m_highlight_all_levels_check_box, 16);
  auto market_check_box_button_group = new QButtonGroup(this);
  market_check_box_button_group->addButton(m_highlight_none_check_box);
  market_check_box_button_group->addButton(m_highlight_top_level_check_box);
  market_check_box_button_group->addButton(m_highlight_all_levels_check_box);
  market_highlight_layout->addStretch(18);
  auto market_highlight_color_label = new QLabel(tr("Highlight Color"), this);
  auto generic_label_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  market_highlight_color_label->setStyleSheet(generic_label_text_style);
  market_highlight_layout->addWidget(market_highlight_color_label, 14);
  market_highlight_layout->addStretch(4);
  m_market_highlight_color_button = new ColorSelectorButton(Qt::yellow, this);
  m_market_highlight_color_button->setFixedWidth(scale_width(100));
  m_market_highlight_color_button->connect_color_signal(
    [=] (auto color) { on_market_highlight_color_selected(color); });
  market_highlight_layout->addWidget(m_market_highlight_color_button, 26);
  market_highlight_layout->addStretch(92);
  layout->addLayout(market_highlight_layout, 130);
  layout->addStretch(18);
  auto vertical_rule = new QWidget(this);
  vertical_rule->setFixedWidth(scale_width(1));
  vertical_rule->setStyleSheet("background-color: #C8C8C8;");
  layout->addWidget(vertical_rule);
  layout->addStretch(18);
  auto orders_layout = new QVBoxLayout();
  orders_layout->setContentsMargins({});
  orders_layout->setSpacing(0);
  auto orders_label = new QLabel(tr("Orders"), this);
  orders_label->setStyleSheet(generic_header_label_stylesheet);
  orders_layout->addWidget(orders_label, 14);
  orders_layout->addStretch(10);
  m_hide_orders_check_box = new CheckBox(tr("Hide Orders"), this);
  m_hide_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(m_hide_orders_check_box, 16);
  orders_layout->addStretch(10);
  m_display_orders_check_box = new CheckBox(tr("Display Orders"), this);
  m_display_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(m_display_orders_check_box, 16);
  orders_layout->addStretch(10);
  m_highlight_orders_check_box = new CheckBox(tr("Highlight Orders"), this);
  m_highlight_orders_check_box->setChecked(true);
  m_highlight_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(m_highlight_orders_check_box, 16);
  auto orders_check_box_button_group = new QButtonGroup(this);
  orders_check_box_button_group->addButton(m_hide_orders_check_box);
  orders_check_box_button_group->addButton(m_display_orders_check_box);
  orders_check_box_button_group->addButton(m_highlight_orders_check_box);
  orders_layout->addStretch(18);
  auto order_highlight_color_label = new QLabel(tr("Highlight Color"), this);
  order_highlight_color_label->setStyleSheet(generic_label_text_style);
  orders_layout->addWidget(order_highlight_color_label, 14);
  orders_layout->addStretch(4);
  m_order_highlight_color_button = new ColorSelectorButton(
    properties.get_order_highlight_color(), this);
  m_order_highlight_color_button->setFixedWidth(scale_width(100));
  orders_layout->addWidget(m_order_highlight_color_button, 26);
  orders_layout->addStretch(92);
  layout->addLayout(orders_layout, 151);
  layout->addStretch(27);
}

void BookViewHighlightPropertiesWidget::apply(
    BookViewProperties& properties) const {
  for(auto i = 0; i < m_markets_list_widget->count(); ++i) {
    auto item = static_cast<MarketListItem*>(m_markets_list_widget->item(i));
    auto& highlight = item->get_market_highlight();
    if(highlight.is_initialized()) {
      properties.set_market_highlight(item->get_market_info().m_code,
        *highlight);
    }
  }
  if(m_highlight_orders_check_box->isChecked()) {
    properties.set_order_highlight(
      BookViewProperties::OrderHighlight::HIGHLIGHT_ORDERS);
  } else if(m_display_orders_check_box->isChecked()) {
    properties.set_order_highlight(
      BookViewProperties::OrderHighlight::DISPLAY_ORDERS);
  } else {
    properties.set_order_highlight(
      BookViewProperties::OrderHighlight::HIDE_ORDERS);
  }
  properties.set_order_highlight_color(
    m_order_highlight_color_button->get_color());
}

void BookViewHighlightPropertiesWidget::showEvent(QShowEvent* event) {
  if(m_markets_list_widget->currentRow() == -1) {
    m_markets_list_widget->setCurrentRow(0);
  }
}

void BookViewHighlightPropertiesWidget::update_market_widgets() {
  auto selected_item = static_cast<MarketListItem*>(
    m_markets_list_widget->currentItem())->get_market_highlight();
  if(selected_item.is_initialized()) {
    m_market_highlight_color_button->set_color(selected_item->m_color);
    if(selected_item->m_highlight_all_levels) {
      m_highlight_all_levels_check_box->setChecked(true);
    } else {
      m_highlight_top_level_check_box->setChecked(true);
    }
  } else {
    m_market_highlight_color_button->set_color(Qt::white);
    m_highlight_none_check_box->setChecked(true);
  }
  update_market_list_stylesheet(m_markets_list_widget->currentRow());
}

void BookViewHighlightPropertiesWidget::update_market_list_stylesheet(
    int selected_item_index) {
  m_markets_list_widget->setStyleSheet(QString(R"(
    QListWidget {
      background-color: white;
      border: 1px solid transparent;
      outline: none;
      padding: %1px %2px 0px %2px;
    }

    QListWidget::item {
      padding-top: %6px;
      padding-bottom: %6px;
    }

    QListWidget::item:selected {
      background-color: %5;
      border: %3px solid #4B23A0 %4px solid #4B23A0;
      color: #000000;
    })").arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(1)).arg(scale_width(1))
        .arg(m_markets_list_widget->item(
          selected_item_index)->background().color().name())
        .arg(scale_height(3)));
  m_markets_list_widget->setMinimumHeight(
    m_markets_list_widget->sizeHintForRow(0) * m_markets_list_widget->count());
}

void BookViewHighlightPropertiesWidget::on_market_highlight_color_selected(
    const QColor& color) {
  auto item = static_cast<MarketListItem*>(
    m_markets_list_widget->currentItem());
  item->set_highlight_color(color);
  item->setBackground(color);
  update_market_widgets();
}

void BookViewHighlightPropertiesWidget::
    on_highlight_none_check_box_checked(int state) {
  if(state == Qt::Checked) {
    auto current_item = static_cast<MarketListItem*>(
      m_markets_list_widget->currentItem());
    current_item->remove_highlight();
    current_item->setBackground(Qt::white);
    update_market_list_stylesheet(m_markets_list_widget->currentRow());
    m_market_highlight_color_button->set_color(Qt::white);
  }
}

void BookViewHighlightPropertiesWidget::
    on_highlight_top_level_check_box_checked(int state) {
  if(state == Qt::Checked) {
    static_cast<MarketListItem*>(m_markets_list_widget->currentItem())->
      set_highlight_top_level();
  }
}

void BookViewHighlightPropertiesWidget::
    on_highlight_all_levels_check_box_checked(int state) {
  if(state == Qt::Checked) {
    static_cast<MarketListItem*>(m_markets_list_widget->currentItem())->
      set_highlight_all_levels();
  }
}
