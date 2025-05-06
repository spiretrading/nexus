#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

QVBoxLayout* Spire::make_vbox_layout(QWidget* parent) {
  auto layout = new QVBoxLayout(parent);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  return layout;
}

QHBoxLayout* Spire::make_hbox_layout(QWidget* parent) {
  auto layout = new QHBoxLayout(parent);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  return layout;
}

QGridLayout* Spire::make_grid_layout(QWidget* parent) {
  auto layout = new QGridLayout(parent);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  return layout;
}

void Spire::enclose(QWidget& parent, QWidget& body) {
  enclose(parent, body, Qt::Alignment());
}

void Spire::enclose(QWidget& parent, QWidget& body, Qt::Alignment alignment) {
  auto layout = make_vbox_layout(&parent);
  layout->setAlignment(alignment);
  layout->addWidget(&body);
}

void Spire::clear(QLayout& layout) {
  while(auto item = layout.takeAt(0)) {
    if(auto sub_layout = item->layout()) {
      clear(*sub_layout);
      delete sub_layout;
    } else if(auto widget = item->widget()) {
      delete widget;
      delete item;
    } else if(auto spacer = item->spacerItem()) {
      delete spacer;
    }
  }
}
