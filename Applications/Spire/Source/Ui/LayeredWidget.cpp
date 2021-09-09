#include "Spire/Ui/LayeredWidget.hpp"
#include <QEvent>

using namespace boost;
using namespace Spire;

namespace {
  void update_size(LayeredWidget& layers, QWidget& widget) {
    auto size = widget.size();
    if(widget.sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag &&
        layers.width() != size.width()) {
      size.setWidth(layers.width());
    }
    if(widget.sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag &&
        layers.height() != size.height()) {
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
  widget->resize(widget->sizeHint());
  update_size(*this, *widget);
}

QSize LayeredWidget::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  m_size_hint.emplace();
  for(auto layer : m_layers) {
    m_size_hint->setWidth(
      std::max(m_size_hint->width(), layer->sizeHint().width()));
    m_size_hint->setHeight(
      std::max(m_size_hint->height(), layer->sizeHint().height()));
  }
  return *m_size_hint;
}

bool LayeredWidget::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    m_size_hint = none;
    updateGeometry();
  }
  return QWidget::event(event);
}

void LayeredWidget::resizeEvent(QResizeEvent* event) {
  for(auto layer : m_layers) {
    update_size(*this, *layer);
  }
}
