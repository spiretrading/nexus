#include "Spire/Ui/LayeredWidget.hpp"
#include <QEvent>

using namespace Spire;

namespace {
  void update_size(LayeredWidget& layers, QWidget& widget) {
    auto size = widget.size();
    if(widget.sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag &&
        layers.width() > size.width() ||
        widget.sizePolicy().horizontalPolicy() & QSizePolicy::ShrinkFlag &&
        layers.width() < size.width()) {
      size.setWidth(layers.width());
    }
    if(widget.sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag &&
        layers.height() > size.height() ||
        widget.sizePolicy().verticalPolicy() & QSizePolicy::ShrinkFlag &&
        layers.height() < size.height()) {
      size.setHeight(layers.height());
    }
    if(size != widget.size()) {
      widget.resize(size);
    }
  }
}

void LayeredWidget::add(QWidget* widget) {
  widget->setParent(this);
  if(!m_layers.empty()) {
    m_layers.back()->stackUnder(widget);
  }
  m_layers.push_back(widget);
  widget->move(0, 0);
  update_size(*this, *widget);
}

QSize LayeredWidget::sizeHint() const {
  auto size = QSize();
  for(auto layer : m_layers) {
    size.setWidth(std::max(size.width(), layer->sizeHint().width()));
    size.setHeight(std::max(size.height(), layer->sizeHint().height()));
  }
  return size;
}

bool LayeredWidget::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    updateGeometry();
  }
  return QWidget::event(event);
}

void LayeredWidget::resizeEvent(QResizeEvent* event) {
  for(auto layer : m_layers) {
    update_size(*this, *layer);
  }
}
