#include "Spire/Ui/LayeredWidget.hpp"

using namespace Spire;

LayeredWidget::LayeredWidget(QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}

void LayeredWidget::add(QWidget* widget) {
  widget->setParent(this);
  if(!m_layers.empty()) {
    m_layers.back()->stackUnder(widget);
  }
  m_layers.push_back(widget);
  widget->setGeometry(0, 0, width(), height());
}

QSize LayeredWidget::sizeHint() const {
  auto size = QSize();
  for(auto layer : m_layers) {
    size.setWidth(std::max(size.width(), layer->sizeHint().width()));
    size.setHeight(std::max(size.height(), layer->sizeHint().height()));
  }
  return size;
}

void LayeredWidget::resizeEvent(QResizeEvent* event) {
  for(auto layer : m_layers) {
    layer->setGeometry(0, 0, width(), height());
  }
}
