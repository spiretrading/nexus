#include "Spire/BookView/BookViewLevelPropertiesWidget.hpp"
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/FontSelectorWidget.hpp"
#include "Spire/Ui/IntegerSpinBox.hpp"
#include "Spire/Ui/ScrollArea.hpp"
#include "Spire/Ui/SpinBoxModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto LABEL_HEIGHT() {
    static auto height = scale_height(14);
    return height;
  }
}

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
  horizontal_layout->addSpacing(scale_width(18));
  auto band_properties_layout = new QVBoxLayout();
  band_properties_layout->setContentsMargins({});
  band_properties_layout->setSpacing(0);
  auto number_of_bands_label = new QLabel(tr("Number of Bands"), this);
  number_of_bands_label->setFixedHeight(LABEL_HEIGHT());
  auto generic_label_style = QString(R"(
    color: #000000;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  number_of_bands_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(number_of_bands_label);
  band_properties_layout->addStretch(4);
  auto number_of_bands_spin_box = new IntegerSpinBox(
    std::make_shared<IntegerSpinBoxModel>(bg_colors.size(), 1,
      std::numeric_limits<int>::max(), 1), this);
  number_of_bands_spin_box->setFixedSize(BUTTON_SIZE());
  number_of_bands_spin_box->connect_change_signal([=] (auto value) {
    on_number_of_bands_spin_box_changed(static_cast<int>(value));
  });
  band_properties_layout->addWidget(number_of_bands_spin_box);
  band_properties_layout->addStretch(10);
  auto band_color_label = new QLabel(tr("Band Color"), this);
  band_color_label->setFixedHeight(LABEL_HEIGHT());
  band_color_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(band_color_label);
  band_properties_layout->addStretch(4);
  m_band_color_button = new ColorSelectorButton(bg_colors[0], this);
  m_band_color_button->connect_color_signal(
    [=] (auto color) { on_band_color_selected(color); });
  m_band_color_button->setFixedSize(BUTTON_SIZE());
  band_properties_layout->addWidget(m_band_color_button);
  band_properties_layout->addStretch(10);
  auto color_gradient_label = new QLabel(tr("Color Gradient"), this);
  color_gradient_label->setFixedHeight(LABEL_HEIGHT());
  color_gradient_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(color_gradient_label);
  band_properties_layout->addStretch(4);
  m_gradient_start_button = new ColorSelectorButton(bg_colors[0], this);
  m_gradient_start_button->setFixedSize(BUTTON_SIZE());
  band_properties_layout->addWidget(m_gradient_start_button);
  band_properties_layout->addStretch(8);
  m_gradient_end_button = new ColorSelectorButton(
    bg_colors[bg_colors.size() - 1], this);
  m_gradient_end_button->setFixedSize(BUTTON_SIZE());
  band_properties_layout->addWidget(m_gradient_end_button);
  band_properties_layout->addStretch(10);
  auto apply_gradient_button = make_flat_button(tr("Apply Gradient"), this);
  apply_gradient_button->setFixedSize(BUTTON_SIZE());
  apply_gradient_button->connect_clicked_signal(
    [=] { on_gradient_apply_button_clicked(); });
  band_properties_layout->addWidget(apply_gradient_button);
  horizontal_layout->addLayout(band_properties_layout);
  horizontal_layout->addSpacing(scale_width(18));
  auto font_layout = new QVBoxLayout();
  font_layout->setContentsMargins({});
  font_layout->setSpacing(0);
  auto change_font_widget = new FontSelectorWidget(
    properties.get_book_quote_font(), this);
  change_font_widget->connect_font_selected_signal(
    [=] (const auto& font) { update_band_list_font(font); });
  change_font_widget->connect_font_preview_signal(
    [=] (const auto& font) { update_band_list_font(font); });
  font_layout->addWidget(change_font_widget);
  font_layout->addSpacing(scale_height(48));
  m_show_grid_lines_check_box = make_check_box(tr("Show Grid"), this);
  m_show_grid_lines_check_box->setChecked(properties.get_show_grid());
  font_layout->addWidget(m_show_grid_lines_check_box);
  font_layout->addStretch(1);
  horizontal_layout->addLayout(font_layout);
  horizontal_layout->addStretch(1);
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
    m_gradient_start_button->get_color().getRgb(&start_red, &start_green,
      &start_blue);
    m_gradient_end_button->get_color().getRgb(&end_red, &end_green, &end_blue);
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
      m_gradient_start_button->get_color());
  }
  update_band_list_stylesheet(m_band_list_widget->currentRow());
}

void BookViewLevelPropertiesWidget::update_band_list_stylesheet(
    int index) {
  if(m_band_list_widget->item(index) != nullptr) {
    m_band_color_button->set_color(
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

void BookViewLevelPropertiesWidget::on_band_color_selected(
    const QColor& color) {
  m_band_list_widget->currentItem()->setBackground(color);
  update_band_list_stylesheet(m_band_list_widget->currentRow());
}

void BookViewLevelPropertiesWidget::on_gradient_apply_button_clicked() {
  update_band_list_gradient();
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
