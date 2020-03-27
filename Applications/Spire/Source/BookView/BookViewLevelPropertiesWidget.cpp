#include "Spire/BookView/BookViewLevelPropertiesWidget.hpp"
#include <QColorDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QSpinBox>
#include <QVBoxLayout>
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace Spire;

BookViewLevelPropertiesWidget::BookViewLevelPropertiesWidget(
    const BookViewProperties& properties, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addStretch(10);
  auto band_appearance_label = new QLabel(tr("Band Appearance"), this);
  band_appearance_label->setFixedHeight(scale_height(14));
  band_appearance_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(12)));
  layout->addWidget(band_appearance_label);
  layout->addStretch(10);
  auto horizontal_layout = new QHBoxLayout();
  horizontal_layout->setContentsMargins({});
  horizontal_layout->setSpacing(0);
  auto band_list_scroll_area = new ScrollArea(this);
  band_list_scroll_area->setFixedSize(scale(140, 222));
  band_list_scroll_area->set_border_style(scale_width(1), QColor("#C8C8C8"));
  band_list_scroll_area->setWidgetResizable(true);
  horizontal_layout->addWidget(band_list_scroll_area, 222);
  m_band_list_widget = new QListWidget(this);
  band_list_scroll_area->setWidget(m_band_list_widget);
  m_band_list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_band_list_stylesheet = QString(R"(
    QListWidget {
      background-color: #FFFFFF;
      border: 1px solid transparent;
      outline: none;
      padding: %1px %2px %1px %2px;
    })").arg(scale_height(4)).arg(scale_width(4));
  m_band_list_widget->setStyleSheet(m_band_list_stylesheet);
  auto& bg_colors =
    properties.get_book_quote_background_colors();
  for(auto i = 0 ; i < static_cast<int>(bg_colors.size()); ++i) {
    auto item = new QListWidgetItem(tr("Level") + QString(" %1").arg(i + 1),
      m_band_list_widget);
    item->setBackground(bg_colors[i]);
    item->setFont(properties.get_book_quote_font());
    item->setTextAlignment(Qt::AlignCenter);
  }
  update_band_list_font(properties.get_book_quote_font());
  m_band_list_widget->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  connect(m_band_list_widget, &QListWidget::currentRowChanged,
    this, &BookViewLevelPropertiesWidget::update_band_list_stylesheet);
  horizontal_layout->addStretch(18);
  auto band_properties_layout = new QVBoxLayout();
  band_properties_layout->setContentsMargins({});
  band_properties_layout->setSpacing(0);
  auto number_of_bands_label = new QLabel(tr("Number of Bands"), this);
  number_of_bands_label->setFixedHeight(scale_height(14));
  auto generic_label_style = QString(R"(
    color: #000000;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  number_of_bands_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(number_of_bands_label);
  band_properties_layout->addStretch(4);
  auto number_of_bands_spin_box = new QSpinBox(this);
  number_of_bands_spin_box->setValue(static_cast<int>(bg_colors.size()));
  number_of_bands_spin_box->setMinimum(1);
  number_of_bands_spin_box->setMaximum(INT_MAX);
  number_of_bands_spin_box->setKeyboardTracking(false);
  number_of_bands_spin_box->setFixedHeight(scale_height(26));
  number_of_bands_spin_box->setStyleSheet(QString(R"(
    QSpinBox {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %5px;
      padding-left: %4px;
    }

    QSpinBox:focus {
      border: %1px solid #4B23A0;
    }

    QSpinBox::up-button {
      border: none;
    }

    QSpinBox::down-button {
      border: none;
    }

    QSpinBox::up-arrow {
      height: %2px;
      image: url(:/Icons/arrow-up.svg);
      padding-top: %6px;
      width: %3px;
    }

    QSpinBox::down-arrow {
      height: %2px;
      image: url(:/Icons/arrow-down.svg);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(6)).arg(scale_width(6))
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(4)));
  connect(number_of_bands_spin_box,
    static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
    &BookViewLevelPropertiesWidget::on_number_of_bands_spin_box_changed);
  band_properties_layout->addWidget(number_of_bands_spin_box);
  band_properties_layout->addStretch(10);
  auto band_color_label = new QLabel(tr("Band Color"), this);
  band_color_label->setFixedHeight(scale_height(14));
  band_color_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(band_color_label);
  band_properties_layout->addStretch(4);
  m_band_color_button = new FlatButton(this);
  m_band_color_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_band_color_button, bg_colors[0]);
  m_band_color_button->connect_clicked_signal(
    [=] { on_band_color_button_clicked(); });
  band_properties_layout->addWidget(m_band_color_button);
  band_properties_layout->addStretch(10);
  auto color_gradient_label = new QLabel(tr("Color Gradient"), this);
  color_gradient_label->setFixedHeight(scale_height(14));
  color_gradient_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(color_gradient_label);
  band_properties_layout->addStretch(4);
  m_gradient_start_button = new FlatButton(this);
  m_gradient_start_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_gradient_start_button, bg_colors[0]);
  m_gradient_start_button->connect_clicked_signal(
    [=] { on_gradient_start_button_clicked(); });
  band_properties_layout->addWidget(m_gradient_start_button);
  band_properties_layout->addStretch(8);
  m_gradient_end_button = new FlatButton(this);
  m_gradient_end_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_gradient_end_button,
    bg_colors[bg_colors.size() - 1]);
  m_gradient_end_button->connect_clicked_signal(
    [=] { on_gradient_end_button_clicked(); });
  band_properties_layout->addWidget(m_gradient_end_button);
  band_properties_layout->addStretch(10);
  auto apply_gradient_button = new FlatButton(tr("Apply Gradient"), this);
  apply_gradient_button->setFixedHeight(scale_height(26));
  QFont generic_button_font;
  generic_button_font.setFamily("Roboto");
  generic_button_font.setPixelSize(scale_height(12));
  apply_gradient_button->setFont(generic_button_font);
  auto generic_button_default_style = apply_gradient_button->get_style();
  generic_button_default_style.m_background_color = QColor("#EBEBEB");
  apply_gradient_button->set_style(generic_button_default_style);
  auto generic_button_hover_style = apply_gradient_button->get_hover_style();
  generic_button_hover_style.m_background_color = QColor("#4B23A0");
  generic_button_hover_style.m_text_color = Qt::white;
  apply_gradient_button->set_hover_style(generic_button_hover_style);
  auto generic_button_focus_style = apply_gradient_button->get_focus_style();
  generic_button_focus_style.m_background_color = QColor("#EBEBEB");
  generic_button_focus_style.m_border_color = QColor("#4B23A0");
  apply_gradient_button->set_focus_style(generic_button_focus_style);
  apply_gradient_button->connect_clicked_signal(
    [=] { on_gradient_apply_button_clicked(); });
  band_properties_layout->addWidget(apply_gradient_button);
  horizontal_layout->addLayout(band_properties_layout);
  horizontal_layout->addStretch(18);
  auto font_button_layout = new QVBoxLayout();
  font_button_layout->setContentsMargins({});
  font_button_layout->setSpacing(0);
  font_button_layout->addStretch(18);
  auto change_font_button = new FlatButton(tr("Change Font"), this);
  change_font_button->setFixedSize(scale(100, 26));
  change_font_button->setFont(generic_button_font);
  change_font_button->set_style(generic_button_default_style);
  change_font_button->set_hover_style(generic_button_hover_style);
  change_font_button->set_focus_style(generic_button_focus_style);
  change_font_button->connect_clicked_signal(
    [=] { on_change_font_button_clicked(); });
  font_button_layout->addWidget(change_font_button);
  font_button_layout->addStretch(33);
  m_show_grid_lines_check_box = new CheckBox(tr("Show Grid Lines"), this);
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
  m_show_grid_lines_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  m_show_grid_lines_check_box->setChecked(properties.get_show_grid());
  font_button_layout->addWidget(m_show_grid_lines_check_box);
  font_button_layout->addStretch(129);
  horizontal_layout->addLayout(font_button_layout);
  horizontal_layout->addStretch(60);
  layout->addLayout(horizontal_layout);
  layout->addStretch(20);
  m_band_list_widget->setCurrentRow(0);
}

void BookViewLevelPropertiesWidget::apply(
    BookViewProperties& properties) const {
  properties.set_book_quote_foreground_color(
    m_band_list_widget->currentItem()->foreground().color());
  properties.set_book_quote_font(m_band_list_widget->currentItem()->font());
  properties.set_show_grid(m_show_grid_lines_check_box->isChecked());
  auto& colors = properties.get_book_quote_background_colors();
  colors.clear();
  for(auto i = 0; i < m_band_list_widget->count(); ++i) {
    colors.push_back(m_band_list_widget->item(i)->background().color());
  }
}

void BookViewLevelPropertiesWidget::showEvent(QShowEvent* event) {
  m_band_list_widget->setFocus();
}

void BookViewLevelPropertiesWidget::set_color_button_stylesheet(
    FlatButton* button, const QColor& color) {
  auto s = button->get_style();
  s.m_background_color = color;
  s.m_border_color = QColor("#C8C8C8");
  button->set_style(s);
  s.m_border_color = QColor("#4B23A0");
  button->set_hover_style(s);
  button->set_focus_style(s);
}

void BookViewLevelPropertiesWidget::update_band_list_font(
    const QFont& font) {
  for(auto i = 0; i < m_band_list_widget->count(); ++i) {
    m_band_list_widget->item(i)->setFont(font);
  }
}

void BookViewLevelPropertiesWidget::update_band_list_gradient() {
  auto band_count = m_band_list_widget->count();
  if(band_count > 1) {
    auto start_red = 0;
    auto start_green = 0;
    auto start_blue = 0;
    auto end_red = 0;
    auto end_green = 0;
    auto end_blue = 0;
    m_gradient_start_button->get_style().m_background_color.getRgb(&start_red,
      &start_green, &start_blue);
    m_gradient_end_button->get_style().m_background_color.getRgb(&end_red,
      &end_green, &end_blue);
    auto red_delta = static_cast<double>((end_red - start_red)) /
      (band_count - 1);
    auto green_delta = static_cast<double>((end_green - start_green)) /
      (band_count -1);
    auto blue_delta = static_cast<double>((end_blue - start_blue)) /
      (band_count - 1);
    auto red = double(start_red);
    auto green = double(start_green);
    auto blue = double(start_blue);
    for(auto i = 0; i < band_count; ++i) {
      m_band_list_widget->item(i)->setBackground(QColor(static_cast<int>(red),
        static_cast<int>(green), static_cast<int>(blue)));
      red += red_delta;
      green += green_delta;
      blue += blue_delta;
    }
  } else {
    m_band_list_widget->item(0)->setBackground(
      m_gradient_start_button->get_style().m_background_color);
  }
  update_band_list_stylesheet(m_band_list_widget->currentRow());
}

void BookViewLevelPropertiesWidget::update_band_list_stylesheet(
    int index) {
  if(m_band_list_widget->item(index) != nullptr) {
    set_color_button_stylesheet(m_band_color_button,
      m_band_list_widget->item(index)->background().color());
    auto selected_stylesheet = QString(R"(
      QListWidget::item:selected {
        background-color: %3;
        border: %1px solid #000000 %2px solid #000000;
        color: #000000;
      })").arg(scale_height(1)).arg(scale_width(1))
          .arg(m_band_list_widget->item(index)->background().color().name());
    m_band_list_widget->setStyleSheet(m_band_list_stylesheet +
      selected_stylesheet);
  }
}

void BookViewLevelPropertiesWidget::on_band_color_button_clicked() {
  auto color = QColorDialog::getColor(
    m_band_color_button->get_style().m_background_color);
  if(color.isValid()) {
    set_color_button_stylesheet(m_band_color_button, color);
    m_band_list_widget->currentItem()->setBackground(color);
    update_band_list_stylesheet(m_band_list_widget->currentRow());
  }
}

void BookViewLevelPropertiesWidget::on_change_font_button_clicked() {
  auto ok = false;
  auto font = QFontDialog::getFont(&ok, m_band_list_widget->item(0)->font());
  if(ok) {
    update_band_list_font(font);
  }
}

void BookViewLevelPropertiesWidget::on_gradient_apply_button_clicked() {
  update_band_list_gradient();
}

void BookViewLevelPropertiesWidget::on_gradient_end_button_clicked() {
  auto color = QColorDialog::getColor(
    m_gradient_end_button->get_style().m_background_color);
  if(color.isValid()) {
    set_color_button_stylesheet(m_gradient_end_button, color);
  }
}

void BookViewLevelPropertiesWidget::on_gradient_start_button_clicked() {
  auto color = QColorDialog::getColor(
    m_gradient_start_button->get_style().m_background_color);
  if(color.isValid()) {
    set_color_button_stylesheet(m_gradient_start_button, color);
  }
}

void BookViewLevelPropertiesWidget::on_number_of_bands_spin_box_changed(
    int value) {
  auto current_row = m_band_list_widget->currentRow();
  auto item_count = m_band_list_widget->count();
  if(value > item_count) {
    for(auto i = item_count; i < value; ++i) {
      auto item = new QListWidgetItem(
        tr("Level") + QString(" %1").arg(i + 1), m_band_list_widget);
      item->setBackground(m_band_list_widget->item(
        item_count - 1)->background());
      item->setFont(m_band_list_widget->item(item_count - 1)->font());
      item->setTextAlignment(Qt::AlignCenter);
    }
  } else {
    for(auto i = item_count; i > value - 1; --i) {
      auto item = m_band_list_widget->takeItem(i);
      delete item;
    }
  }
  if(current_row > m_band_list_widget->count() - 1) {
    m_band_list_widget->setCurrentRow(m_band_list_widget->count() - 1);
  } else {
    m_band_list_widget->setCurrentRow(current_row);
  }
  m_band_list_widget->setFixedHeight(m_band_list_widget->sizeHintForRow(0) *
    (m_band_list_widget->count() + 1));
}
