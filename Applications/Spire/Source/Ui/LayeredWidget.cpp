#include "Spire/Ui/LayeredWidget.hpp"

using namespace Spire;

void LayeredWidget::add(QWidget* widget) {
  widget->setParent(this);
  if(!m_layers.empty()) {
    m_layers.back()->stackUnder(widget);
  }
  m_layers.push_back(widget);
  widget->setGeometry(0, 0, width(), height());
}

void LayeredWidget::resizeEvent(QResizeEvent* event) {
  for(auto layer : m_layers) {
    layer->setGeometry(0, 0, width(), height());
  }
}
