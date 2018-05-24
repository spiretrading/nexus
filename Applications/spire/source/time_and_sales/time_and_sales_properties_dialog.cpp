#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include <QColorDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/ui/check_box.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;
using price_range = time_and_sales_properties::price_range;
using columns = time_and_sales_properties::columns;

time_and_sales_properties_dialog::time_and_sales_properties_dialog(
    const time_and_sales_properties& properties, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags | Qt::Window | Qt::FramelessWindowHint |
        Qt::WindowCloseButtonHint) {
  m_body = new QWidget(this);
  m_body->setFixedSize(scale(492, 312));
  m_body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  window->set_icon(imageFromSvg(":/icons/time-sale-black.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))),
    imageFromSvg(":/icons/time-sale-grey.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))));
  this->window()->setWindowTitle(tr("Properties"));
  window_layout->addWidget(window);
  m_body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins(scale_width(8), scale_height(10), scale_width(8),
    scale_height(8));
  layout->setSpacing(0);
  auto style_layout = new QHBoxLayout();
  style_layout->setContentsMargins({});
  style_layout->setSpacing(0);
  auto band_list_layout = new QVBoxLayout();
  band_list_layout->setContentsMargins({});
  band_list_layout->setSpacing(0);
  auto section_label_style = QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(12));
  auto band_appearance_label = new QLabel(tr("Band Appearance"), this);
  band_appearance_label->setStyleSheet(section_label_style);
  band_list_layout->addWidget(band_appearance_label);
  band_list_layout->setStretchFactor(band_appearance_label, 14);
  band_list_layout->addStretch(10);
  m_band_list = new QListWidget(this);
  m_band_list->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  m_band_list->setSelectionBehavior(
    QAbstractItemView::SelectionBehavior::SelectRows);
  connect(m_band_list, &QListWidget::currentRowChanged,
    [=] (auto index) {update_colors(index);});
  auto band_unknown_item = new QListWidgetItem(tr("Bid/Ask Unknown"),
    m_band_list);
  band_unknown_item->setTextAlignment(Qt::AlignCenter);
  auto above_ask_item = new QListWidgetItem(tr("Trade Above Ask"),
    m_band_list);
  above_ask_item->setTextAlignment(Qt::AlignCenter);
  auto at_ask_item = new QListWidgetItem(tr("Trade At Ask"),
    m_band_list);
  at_ask_item->setTextAlignment(Qt::AlignCenter);
  auto inside_item = new QListWidgetItem(tr("Trade Inside"),
    m_band_list);
  inside_item->setTextAlignment(Qt::AlignCenter);
  auto at_bid_item = new QListWidgetItem(tr("Trade At Bid"),
    m_band_list);
  at_bid_item->setTextAlignment(Qt::AlignCenter);
  auto below_bid_item = new QListWidgetItem(tr("Trade Below Bid"),
    m_band_list);
  below_bid_item->setTextAlignment(Qt::AlignCenter);
  m_band_list_stylesheet = QString(R"(
    QListWidget {
      background-color: white;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      font-family: Roboto;
      font-size: %5px;
      outline: none;
      padding: %3px %4px 0px %4px;
    }

    QListWidget::item {
      height: %6px;
    })").arg(scale_height(1)).arg(scale_width(1))
        .arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(11)).arg(scale_height(16));
  m_band_list->setItemSelected(band_unknown_item, true);
  band_list_layout->addWidget(m_band_list);
  band_list_layout->setStretchFactor(m_band_list, 120);
  style_layout->addLayout(band_list_layout);
  style_layout->setStretchFactor(band_list_layout, 144);
  style_layout->addStretch(10);
  auto color_settings_layout = new QVBoxLayout();
  color_settings_layout->setContentsMargins({});
  color_settings_layout->setSpacing(0);
  color_settings_layout->addStretch(24);
  auto text_color_label = new QLabel(tr("Text Color"), this);
  auto generic_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  text_color_label->setStyleSheet(generic_text_style);
  color_settings_layout->addWidget(text_color_label);
  color_settings_layout->setStretchFactor(text_color_label, 14);
  color_settings_layout->addStretch(4);
  m_text_color_button = new flat_button(this);
  m_text_color_button->connect_clicked_signal(
    [=] { set_text_color(); });
  set_color_button_stylesheet(m_text_color_button,
    properties.get_text_color(
    price_range::UNKNOWN));
  color_settings_layout->addWidget(m_text_color_button);
  color_settings_layout->setStretchFactor(m_text_color_button, 20);
  color_settings_layout->addStretch(10);
  auto band_color_label = new QLabel(tr("Band Color"), this);
  band_color_label->setStyleSheet(generic_text_style);
  color_settings_layout->addWidget(band_color_label);
  color_settings_layout->setStretchFactor(band_color_label, 14);
  color_settings_layout->addStretch(4);
  m_band_color_button = new flat_button(this);
  m_band_color_button->connect_clicked_signal(
    [=] { set_band_color(); });
  set_color_button_stylesheet(m_band_color_button, properties.get_band_color(
    price_range::UNKNOWN));
  color_settings_layout->addWidget(m_band_color_button);
  color_settings_layout->setStretchFactor(m_band_color_button, 20);
  color_settings_layout->addStretch(18);
  m_show_grid_check_box = new check_box(tr("Show Grid"), this);
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
    image: url(:/icons/check-with-box.svg);)");
  auto check_box_hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto check_box_focused_style = QString(R"(border-color: #4B23A0;)");
  m_show_grid_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  color_settings_layout->addWidget(m_show_grid_check_box);
  color_settings_layout->setStretchFactor(m_show_grid_check_box, 16);
  style_layout->addLayout(color_settings_layout);
  style_layout->setStretchFactor(color_settings_layout, 80);
  style_layout->addStretch(40);
  auto font_layout = new QVBoxLayout();
  font_layout->setContentsMargins({});
  font_layout->setSpacing(0);
  auto font_label = new QLabel(tr("Font"), this);
  font_label->setStyleSheet(section_label_style);
  font_layout->addWidget(font_label);
  font_layout->setStretchFactor(font_label, 14);
  font_layout->addStretch(10);
  m_font_preview_label = new QLabel(tr("Aa Bb Cc\n0123"), this);
  m_font_preview_label->setSizePolicy(QSizePolicy::Ignored,
    QSizePolicy::Ignored);
  m_font_preview_label->setAlignment(Qt::AlignCenter);
  font_layout->addWidget(m_font_preview_label);
  font_layout->setStretchFactor(m_font_preview_label, 84);
  font_layout->addStretch(10);
  auto edit_font_button = new flat_button(tr("Edit Font"), this);
  edit_font_button->connect_clicked_signal([=] { set_font(); });
  auto generic_button_default_style = QString(R"(
    background-color: #EBEBEB;
    color: black;
    font-family: Roboto;
    font-size: %1px;
    qproperty-alignment: AlignCenter;)").arg(scale_height(12));
  auto generic_button_hover_style = QString(R"(
    background-color: #4B23A0;
    color: white;)");
  auto generic_button_focused_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  edit_font_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  font_layout->addWidget(edit_font_button);
  font_layout->setStretchFactor(edit_font_button, 26);
  style_layout->addLayout(font_layout);
  style_layout->setStretchFactor(font_layout, 120);
  style_layout->addStretch(86);
  layout->addLayout(style_layout);
  layout->addStretch(20);
  layout->setStretchFactor(style_layout, 144);
  auto column_settings_layout = new QVBoxLayout();
  column_settings_layout->setContentsMargins({});
  column_settings_layout->setSpacing(0);
  auto column_label = new QLabel(tr("Column"), this);
  column_label->setStyleSheet(section_label_style);
  column_settings_layout->addWidget(column_label);
  column_settings_layout->setStretchFactor(column_label, 14);
  column_settings_layout->addStretch(10);
  auto column_check_box_layout = new QHBoxLayout();
  column_check_box_layout->setContentsMargins({});
  column_check_box_layout->setSpacing(0);
  m_time_check_box = new check_box(tr("Time"), this);
  m_time_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  column_check_box_layout->addWidget(m_time_check_box);
  column_check_box_layout->addStretch(20);
  m_price_check_box = new check_box(tr("Price"), this);
  m_price_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  column_check_box_layout->addWidget(m_price_check_box);
  column_check_box_layout->addStretch(20);
  m_market_check_box = new check_box(tr("Market"), this);
  m_market_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  column_check_box_layout->addWidget(m_market_check_box);
  column_check_box_layout->addStretch(20);
  m_size_check_box = new check_box(tr("Size"), this);
  m_size_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  column_check_box_layout->addWidget(m_size_check_box);
  column_check_box_layout->addStretch(20);
  m_condition_check_box = new check_box(tr("Condition"), this);
  m_condition_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  column_check_box_layout->addWidget(m_condition_check_box);
  column_check_box_layout->addStretch(128);
  column_settings_layout->addLayout(column_check_box_layout);
  column_settings_layout->setStretchFactor(column_check_box_layout, 16);
  layout->addLayout(column_settings_layout);
  layout->setStretchFactor(column_settings_layout, 40);
  layout->addStretch(30);
  auto buttons_layout = new QHBoxLayout();
  buttons_layout->setContentsMargins({});
  buttons_layout->setSpacing(0);
  auto buttons_layout_1 = new QVBoxLayout();
  buttons_layout_1->setContentsMargins({});
  buttons_layout_1->setSpacing(0);
  auto save_as_default_button = new flat_button(tr("Save As Default"), this);
  save_as_default_button->connect_clicked_signal(
    [=] { m_save_default_signal(m_properties); });
  save_as_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_1->addWidget(save_as_default_button);
  buttons_layout_1->setStretchFactor(save_as_default_button, 26);
  buttons_layout_1->addStretch(8);
  auto load_default_button = new flat_button(tr("Load Default"), this);
  load_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_1->addWidget(load_default_button);
  buttons_layout_1->setStretchFactor(load_default_button, 26);
  buttons_layout->addLayout(buttons_layout_1);
  buttons_layout->setStretchFactor(buttons_layout_1, 100);
  buttons_layout->addStretch(8);
  auto buttons_layout_2 = new QVBoxLayout();
  buttons_layout_2->setContentsMargins({});
  buttons_layout_2->setSpacing(0);
  buttons_layout_2->addStretch(34);
  auto reset_default_button = new flat_button(tr("Reset Default"), this);
  reset_default_button->connect_clicked_signal(
    [=] { m_save_default_signal(time_and_sales_properties()); });
  reset_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_2->addWidget(reset_default_button);
  buttons_layout_2->setStretchFactor(reset_default_button, 26);
  buttons_layout->addLayout(buttons_layout_2);
  buttons_layout->setStretchFactor(buttons_layout_2, 100);
  buttons_layout->addStretch(60);
  auto buttons_layout_3 = new QVBoxLayout();
  buttons_layout_3->setContentsMargins({});
  buttons_layout_3->setSpacing(0);
  auto apply_to_all_button = new flat_button(tr("Apply To All"), this);
  apply_to_all_button->connect_clicked_signal(
    [=] { m_apply_all_signal(m_properties); });
  apply_to_all_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_3->addWidget(apply_to_all_button);
  buttons_layout_3->setStretchFactor(apply_to_all_button, 26);
  buttons_layout_3->addStretch(8);
  auto cancel_button = new flat_button(tr("Cancel"), this);
  cancel_button->connect_clicked_signal(
    [=] { reject(); });
  cancel_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_3->addWidget(cancel_button);
  buttons_layout_3->setStretchFactor(cancel_button, 26);
  buttons_layout->addLayout(buttons_layout_3);
  buttons_layout->setStretchFactor(buttons_layout_3, 100);
  buttons_layout->addStretch(8);
  auto buttons_layout_4 = new QVBoxLayout();
  buttons_layout_4->setContentsMargins({});
  buttons_layout_4->setSpacing(0);
  auto apply_button = new flat_button(tr("Apply"), this);
  setTabOrder(apply_to_all_button, apply_button);
  apply_button->connect_clicked_signal(
    [=] { m_apply_signal(m_properties); });
  apply_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_4->addWidget(apply_button);
  buttons_layout_4->setStretchFactor(apply_button, 26);
  buttons_layout_4->addStretch(8);
  auto ok_button = new flat_button(tr("OK"), this);
  ok_button->connect_clicked_signal(
    [=] { accept(); });
  ok_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  buttons_layout_4->addWidget(ok_button);
  buttons_layout_4->setStretchFactor(ok_button, 26);
  buttons_layout->addLayout(buttons_layout_4);
  buttons_layout->setStretchFactor(buttons_layout_4, 100);
  layout->addLayout(buttons_layout);
  layout->setStretchFactor(buttons_layout, 60);
  set_properties(properties);
}

time_and_sales_properties
    time_and_sales_properties_dialog::get_properties() const {
  time_and_sales_properties properties;
  auto unknown_item = m_band_list->item(0);
  properties.set_band_color(price_range::UNKNOWN,
    unknown_item->backgroundColor());
  properties.set_text_color(price_range::UNKNOWN,
    unknown_item->textColor());
  auto above_ask_item = m_band_list->item(1);
  properties.set_band_color(price_range::ABOVE_ASK,
    above_ask_item->backgroundColor());
  properties.set_text_color(price_range::ABOVE_ASK,
    above_ask_item->textColor());
  auto at_ask_item = m_band_list->item(2);
  properties.set_band_color(price_range::AT_ASK,
    at_ask_item->backgroundColor());
  properties.set_text_color(price_range::AT_ASK,
    at_ask_item->textColor());
  auto inside_item = m_band_list->item(3);
  properties.set_band_color(price_range::INSIDE,
    inside_item->backgroundColor());
  properties.set_text_color(price_range::INSIDE,
    inside_item->textColor());
  auto at_bid_item = m_band_list->item(4);
  properties.set_band_color(price_range::AT_BID,
    at_bid_item->backgroundColor());
  properties.set_text_color(price_range::AT_BID,
    at_bid_item->textColor());
  auto below_bid_item = m_band_list->item(5);
  properties.set_band_color(price_range::BELOW_BID,
    below_bid_item->backgroundColor());
  properties.set_text_color(price_range::BELOW_BID,
    below_bid_item->textColor());
  if(m_show_grid_check_box->isChecked()) {
    properties.m_show_grid = true;
  } else {
    properties.m_show_grid = false;
  }
  properties.m_font = m_font_preview_label->font();
  if(m_time_check_box->isChecked()) {
    properties.set_show_column(columns::TIME_COLUMN, true);
  } else {
    properties.set_show_column(columns::TIME_COLUMN, false);
  }
  if(m_price_check_box->isChecked()) {
    properties.set_show_column(columns::PRICE_COLUMN, true);
  } else {
    properties.set_show_column(columns::PRICE_COLUMN, false);
  }
  if(m_market_check_box->isChecked()) {
    properties.set_show_column(columns::MARKET_COLUMN, true);
  } else {
    properties.set_show_column(columns::MARKET_COLUMN, false);
  }
  if(m_size_check_box->isChecked()) {
    properties.set_show_column(columns::SIZE_COLUMN, true);
  } else {
    properties.set_show_column(columns::SIZE_COLUMN, false);
  }
  if(m_condition_check_box->isChecked()) {
    properties.set_show_column(columns::CONDITION_COLUMN, true);
  } else {
    properties.set_show_column(columns::CONDITION_COLUMN, false);
  }
  return properties;
}

connection time_and_sales_properties_dialog::connect_apply_signal(
    const apply_signal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection time_and_sales_properties_dialog::connect_apply_all_signal(
    const apply_all_signal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection time_and_sales_properties_dialog::connect_save_default_signal(
    const save_default_signal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}

void time_and_sales_properties_dialog::mousePressEvent(QMouseEvent* event) {
  setFocus();
}

void time_and_sales_properties_dialog::showEvent(QShowEvent* event) {
  m_band_list->setFocus();
  auto parent_geometry = static_cast<QWidget*>(parent())->geometry();
  move(parent_geometry.center().x() - (width() / 2),
    parent_geometry.center().y() - (height() / 2));
}

void time_and_sales_properties_dialog::set_band_color() {
  auto index = m_band_list->currentIndex().row();
  auto band = static_cast<price_range>(index);
  auto current_color = m_properties.get_band_color(band);
  auto color = QColorDialog::getColor(current_color);
  if(color.isValid()) {
    m_properties.set_band_color(band, color);
    m_band_list->item(index)->setBackground(color);
    update_colors(index);
  }
}

void time_and_sales_properties_dialog::set_font() {
  bool ok = false;
  auto font = QFontDialog::getFont(&ok, m_properties.m_font);
  if(ok) {
    m_font_preview_label->setFont(font);
    m_properties.m_font = font;
    update_font_preview_stylesheet();
  }
}

void time_and_sales_properties_dialog::set_text_color() {
  auto index = m_band_list->currentIndex().row();
  auto band = static_cast<price_range>(index);
  auto current_color = m_properties.get_text_color(band);
  auto color = QColorDialog::getColor(current_color);
  if(color.isValid()) {
    m_properties.set_text_color(band, color);
    m_band_list->item(index)->setForeground(color);
    update_colors(index);
  }
}

void time_and_sales_properties_dialog::set_color_button_stylesheet(
    flat_button* button, const QColor& color) {
  button->set_stylesheet(QString(R"(
    background-color: %1;
    border: %2 solid #C8C8C8 %3 solid #C8C8C8;)")
    .arg(color.name()).arg(scale_height(1)).arg(scale_width(1)),
    QString(R"(border: %4 solid #4B23A0 %5 solid #4B23A0;)")
      .arg(scale_height(1)).arg(scale_width(1)),
    QString(R"(border: %4 solid #4B23A0 %5 solid #4B23A0;)")
      .arg(scale_height(1)).arg(scale_width(1)), "");
}

void time_and_sales_properties_dialog::set_color_settings_stylesheet(
    int band_index) {
  auto i = static_cast<price_range>(band_index);
  set_color_button_stylesheet(m_band_color_button,
    m_properties.get_band_color(i));
  set_color_button_stylesheet(m_text_color_button,
    m_properties.get_text_color(i));
}

void time_and_sales_properties_dialog::set_properties(
    const time_and_sales_properties& properties) {
  m_properties = properties;
  auto unknown_item = m_band_list->item(0);
  unknown_item->setBackground(m_properties.get_band_color(
    price_range::UNKNOWN));
  unknown_item->setForeground(m_properties.get_text_color(
    price_range::UNKNOWN));
  auto above_ask_item = m_band_list->item(1);
  above_ask_item->setBackground(m_properties.get_band_color(
    price_range::ABOVE_ASK));
  above_ask_item->setForeground(m_properties.get_text_color(
    price_range::ABOVE_ASK));
  auto at_ask_item = m_band_list->item(2);
  at_ask_item->setBackground(m_properties.get_band_color(
    price_range::AT_ASK));
  at_ask_item->setForeground(m_properties.get_text_color(
    price_range::AT_ASK));
  auto inside_item = m_band_list->item(3);
  inside_item->setBackground(m_properties.get_band_color(
    price_range::INSIDE));
  inside_item->setForeground(m_properties.get_text_color(
    price_range::INSIDE));
  auto at_bid_item = m_band_list->item(4);
  at_bid_item->setBackground(m_properties.get_band_color(
    price_range::AT_BID));
  at_bid_item->setForeground(m_properties.get_text_color(
    price_range::AT_BID));
  auto below_bid_item = m_band_list->item(5);
  below_bid_item->setBackground(m_properties.get_band_color(
    price_range::BELOW_BID));
  below_bid_item->setForeground(m_properties.get_text_color(
    price_range::BELOW_BID));
  set_color_settings_stylesheet(m_band_list->currentRow());
  m_show_grid_check_box->setChecked(m_properties.m_show_grid);
  m_font_preview_label->setFont(m_properties.m_font);
  update_font_preview_stylesheet();
  m_time_check_box->setChecked(m_properties.get_show_column(
    columns::TIME_COLUMN));
  m_price_check_box->setChecked(m_properties.get_show_column(
    columns::PRICE_COLUMN));
  m_market_check_box->setChecked(m_properties.get_show_column(
    columns::MARKET_COLUMN));
  m_size_check_box->setChecked(m_properties.get_show_column(
    columns::SIZE_COLUMN));
  m_condition_check_box->setChecked(m_properties.get_show_column(
    columns::CONDITION_COLUMN));
}

void time_and_sales_properties_dialog::update_colors(int band_index) {
  set_color_settings_stylesheet(band_index);
  auto i = static_cast<price_range>(band_index);
  auto selected_stylesheet = QString(R"(
    QListWidget::item:selected {
      background-color: %3;
      border: %1px solid #4B23A0 %2px solid #4B23A0;
      color: %4;
    })").arg(scale_height(1)).arg(scale_width(1))
    .arg(m_properties.get_band_color(i).name())
    .arg(m_properties.get_text_color(i).name());
  m_band_list->setStyleSheet(m_band_list_stylesheet + selected_stylesheet);
}

void time_and_sales_properties_dialog::update_font_preview_stylesheet() {
  m_font_preview_label->setStyleSheet(QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    )").arg(scale_height(1)).arg(scale_width(1)));
}
