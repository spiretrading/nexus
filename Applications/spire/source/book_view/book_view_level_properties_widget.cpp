#include "spire/book_view/book_view_level_properties_widget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/flat_button.hpp"

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
  layout->addWidget(band_appearance_label);
  layout->addStretch(10);
  auto horizontal_layout = new QHBoxLayout();
  horizontal_layout->setContentsMargins({});
  horizontal_layout->setSpacing(0);
  auto band_list_widget = new QListWidget(this);
  band_list_widget->setFixedSize(scale(140, 222));
  horizontal_layout->addWidget(band_list_widget);
  horizontal_layout->addStretch(18);
  auto band_properties_layout = new QVBoxLayout();
  band_properties_layout->setContentsMargins({});
  band_properties_layout->setSpacing(0);
  auto number_of_bands_label = new QLabel(tr("Number of Bands"), this);
  number_of_bands_label->setFixedHeight(scale_height(14));
  band_properties_layout->addWidget(number_of_bands_label);
  horizontal_layout->addLayout(band_properties_layout);
  horizontal_layout->addStretch(18);
  auto font_button_layout = new QVBoxLayout();
  font_button_layout->setContentsMargins({});
  font_button_layout->setSpacing(0);
  font_button_layout->addStretch(18);
  auto change_font_button = new flat_button(tr("Change Font"), this);
  change_font_button->setFixedSize(scale(100, 26));
  font_button_layout->addWidget(change_font_button);
  horizontal_layout->addLayout(font_button_layout);
  layout->addLayout(horizontal_layout);
  layout->addStretch(20);
}

void book_view_level_properties_widget::apply(
    book_view_properties& properties) const {
}
