#include "spire/book_view/book_view_level_properties_widget.hpp"
#include <QColorDialog>
#include <QFontDialog>
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
  m_band_list_widget->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
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
  auto bg_colors =
    properties.get_book_quote_background_colors();
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
  connect(number_of_bands_spin_box,
    static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
    &book_view_level_properties_widget::on_number_of_bands_spin_box_changed);
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
  m_band_color_button->connect_clicked_signal(
    [=] { on_band_color_button_clicked(); });
  band_properties_layout->addWidget(m_band_color_button);
  band_properties_layout->addStretch(10);
  auto color_gradient_label = new QLabel(tr("Color Gradient"), this);
  color_gradient_label->setFixedHeight(scale_height(14));
  color_gradient_label->setStyleSheet(generic_label_style);
  band_properties_layout->addWidget(color_gradient_label);
  band_properties_layout->addStretch(4);
  m_gradient_start_button = new flat_button(this);
  m_gradient_start_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_gradient_start_button, bg_colors[0]);
  m_gradient_start_button->connect_clicked_signal(
    [=] { on_gradient_start_button_clicked(); });
  band_properties_layout->addWidget(m_gradient_start_button);
  band_properties_layout->addStretch(8);
  m_gradient_end_button = new flat_button(this);
  m_gradient_end_button->setFixedHeight(scale_height(26));
  set_color_button_stylesheet(m_gradient_end_button,
    bg_colors[bg_colors.size() - 1]);
  m_gradient_end_button->connect_clicked_signal(
    [=] { on_gradient_end_button_clicked(); });
  band_properties_layout->addWidget(m_gradient_end_button);
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
  apply_gradient_button->connect_clicked_signal(
    [=] { on_gradient_apply_button_clicked(); });
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
  change_font_button->connect_clicked_signal(
    [=] { on_change_font_button_clicked(); });
  font_button_layout->addWidget(change_font_button);
  font_button_layout->addStretch(33);
  m_show_grid_lines_check_box = new check_box(tr("Show Grid Lines"), this);
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
  m_show_grid_lines_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  font_button_layout->addWidget(m_show_grid_lines_check_box);
  font_button_layout->addStretch(129);
  horizontal_layout->addLayout(font_button_layout);
  horizontal_layout->addStretch(60);
  layout->addLayout(horizontal_layout);
  layout->addStretch(20);
  populate_band_list(static_cast<int>(bg_colors.size()));
  update_band_list_font(properties.get_book_quote_font());
  update_band_list_gradient();
  m_band_list_widget->setCurrentRow(0);
}

void book_view_level_properties_widget::apply(
    book_view_properties& properties) const {
  properties.set_book_quote_foreground_color(
    m_band_list_widget->currentItem()->textColor());
  properties.set_book_quote_font(m_band_list_widget->currentItem()->font());
  properties.set_show_grid(m_show_grid_lines_check_box->isChecked());
  auto& colors = properties.get_book_quote_background_colors();
  colors.clear();
  for(auto i = 0; i < m_band_list_widget->count(); ++i) {
    colors.push_back(m_band_list_widget->item(i)->backgroundColor());
  }
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

void book_view_level_properties_widget::populate_band_list(int num_items) {
  m_band_list_widget->clear();
  for(auto i = 0 ; i < num_items; ++i) {
    auto item = new QListWidgetItem(tr("Level") + QString(" %1").arg(i + 1),
      m_band_list_widget);
    item->setTextAlignment(Qt::AlignCenter);
  }
}

void book_view_level_properties_widget::update_band_list_font(
    const QFont& font) {
  for(auto i = 0; i < m_band_list_widget->count(); ++i) {
    m_band_list_widget->item(i)->setFont(font);
  }
}

void book_view_level_properties_widget::update_band_list_gradient() {
  auto band_count = m_band_list_widget->count();
  int start_red;
  int start_green;
  int start_blue;
  int end_red;
  int end_green;
  int end_blue;
  m_gradient_start_button->palette().background().color().getRgb(&start_red,
    &start_green, &start_blue);
  m_gradient_end_button->palette().background().color().getRgb(&end_red,
    &end_green, &end_blue);
  for(auto i = 0; i < band_count; ++i) {
    auto red = start_red + (i * ((end_red - start_red) / band_count));
    auto green = start_green + (i * ((end_green - start_green) / band_count));
    auto blue = start_blue + (i * ((end_blue - start_blue) / band_count));
    m_band_list_widget->item(i)->setBackground(QColor(red, green, blue));
  }
}

void book_view_level_properties_widget::update_band_list_stylesheet(
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

void book_view_level_properties_widget::on_band_color_button_clicked() {
  auto color = QColorDialog::getColor(m_band_color_button->palette().color(
    QPalette::Window));
  if(color.isValid()) {
    set_color_button_stylesheet(m_band_color_button, color);
    m_band_list_widget->currentItem()->setBackground(color);
    update_band_list_stylesheet(m_band_list_widget->currentRow());
  }
}

void book_view_level_properties_widget::on_change_font_button_clicked() {
  auto ok = false;
  auto font = QFontDialog::getFont(&ok, m_band_list_widget->item(0)->font());
  if(ok) {
    update_band_list_font(font);
  }
}

void book_view_level_properties_widget::on_gradient_apply_button_clicked() {
  update_band_list_gradient();
  set_color_button_stylesheet(m_band_color_button, Qt::yellow);
}

void book_view_level_properties_widget::on_gradient_end_button_clicked() {
  auto color = QColorDialog::getColor(m_gradient_end_button->palette().color(
    QPalette::Window));
  if(color.isValid()) {
    set_color_button_stylesheet(m_gradient_end_button, color);
  }
}

void book_view_level_properties_widget::on_gradient_start_button_clicked() {
  auto color = QColorDialog::getColor(m_gradient_start_button->palette().color(
    QPalette::Window));
  if(color.isValid()) {
    set_color_button_stylesheet(m_gradient_start_button, color);
  }
}

void book_view_level_properties_widget::on_number_of_bands_spin_box_changed(
    int value) {
  populate_band_list(value);
  update_band_list_gradient();
}
