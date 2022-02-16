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

void Spire::enclose(QWidget& parent, QWidget& body) {
  enclose(parent, body, Qt::AlignCenter);
}

void Spire::enclose(QWidget& parent, QWidget& body, Qt::Alignment alignment) {
  auto layout = make_vbox_layout(&parent);
  layout->setAlignment(alignment);
  layout->addWidget(&body);
}
