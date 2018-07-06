#include "spire/book_view/book_view_level_properties_widget.hpp"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QSpinBox>
#include <QVBoxLayout>
#include "spire/book_view/book_view_properties.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/check_box.hpp"

using namespace spire;

book_view_level_properties_widget::book_view_level_properties_widget(
    const book_view_properties& properties, QWidget* parent)
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
    font-size: %1px;)").arg(scale_height(12)));
  layout->addWidget(band_appearance_label);
  layout->addStretch(10);
  auto horizontal_layout = new QHBoxLayout();
  horizontal_layout->setContentsMargins({});
  horizontal_layout->setSpacing(0);
  m_band_list_widget = new QListWidget(this);
  m_band_list_widget->setFixedSize(scale(140, 222));
  m_band_list_stylesheet = QString(R"(
    QListWidget {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      outline: none;
      padding: %3px %4px %3px %4px;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(4))
        .arg(scale_width(4));
  m_band_list_widget->setStyleSheet(m_band_list_stylesheet);
  auto bg_colors =
    properties.get_book_quote_background_colors();
  for(auto i = 0 ; i < bg_colors.size(); ++i) {
    auto item = new QListWidgetItem(tr("Level") + QString(" %1").arg(i + 1),
      m_band_list_widget);
    item->setBackground(bg_colors[i]);
    item->setForeground(properties.get_book_quote_foreground_color());
    item->setFont(properties.get_book_quote_font());
    item->setTextAlignment(Qt::AlignCenter);
  }
  m_band_list_widget->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  m_band_list_widget->setItemSelected(m_band_list_widget->item(0), true);
  connect(m_band_list_widget, &QListWidget::currentRowChanged,
    this, &book_view_level_properties_widget::update_band_list_stylesheet);
  horizontal_layout->addWidget(m_band_list_widget);
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
      image: url(:/icons/arrow-up.svg);
      width: %3px;
    }

    QSpinBox::down-arrow {
      height: %2px;
      image: url(:/icons/arrow-down.svg);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(6)).arg(scale_width(6))
        .arg(scale_width(10)).arg(scale_height(12)));
  band_properties_layout->addWidget(number_of_bands_spin_box);
  band_properties_layout->addStretch(10);
  auto band_color_label = new QLabel(tr("Band Color"), this);
  band_color_label->setFixedHeight(scale_height(14));
  band_color_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(band_color_label);
  band_properties_layout->addStretch(4);
  m_band_color_button = new flat_button(this);
  m_band_color_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_band_color_button, bg_colors[0]);
  band_properties_layout->addWidget(m_band_color_button);
  band_properties_layout->addStretch(10);
  auto color_gradient_label = new QLabel(tr("Color Gradient"), this);
  color_gradient_label->setFixedHeight(scale_height(14));
  color_gradient_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(color_gradient_label);
  band_properties_layout->addStretch(4);
  auto gradient_start_button = new flat_button(this);
  gradient_start_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(gradient_start_button, QColor(Qt::red));
  band_properties_layout->addWidget(gradient_start_button);
  band_properties_layout->addStretch(8);
  auto gradient_end_button = new flat_button(this);
  gradient_end_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(gradient_end_button, QColor(Qt::red));
  band_properties_layout->addWidget(gradient_end_button);
  band_properties_layout->addStretch(10);
  auto apply_gradient_button = new flat_button(tr("Apply Gradient"), this);
  apply_gradient_button->setFixedHeight(scale_height(26));
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
  apply_gradient_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  band_properties_layout->addWidget(apply_gradient_button);
  horizontal_layout->addLayout(band_properties_layout);
  horizontal_layout->addStretch(18);
  auto font_button_layout = new QVBoxLayout();
  font_button_layout->setContentsMargins({});
  font_button_layout->setSpacing(0);
  font_button_layout->addStretch(18);
  auto change_font_button = new flat_button(tr("Change Font"), this);
  change_font_button->setFixedSize(scale(100, 26));
  change_font_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  font_button_layout->addWidget(change_font_button);
  font_button_layout->addStretch(33);
  auto show_grid_lines_check_box = new check_box(tr("Show Grid Lines"), this);
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
  show_grid_lines_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  font_button_layout->addWidget(show_grid_lines_check_box);
  font_button_layout->addStretch(129);
  horizontal_layout->addLayout(font_button_layout);
  horizontal_layout->addStretch(60);
  layout->addLayout(horizontal_layout);
  layout->addStretch(20);
}

void book_view_level_properties_widget::apply(
    book_view_properties& properties) const {
}

void book_view_level_properties_widget::showEvent(QShowEvent* event) {
  m_band_list_widget->setFocus();
}

void book_view_level_properties_widget::set_color_button_stylesheet(
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

void book_view_level_properties_widget::update_band_list_stylesheet(
    int index) {
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
