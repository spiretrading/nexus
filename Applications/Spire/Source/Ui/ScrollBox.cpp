#include "Spire/Ui/ScrollBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace Spire;

ScrollBox::ScrollBox(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  m_layers = new LayeredWidget(this);
  auto container = new QWidget();
  m_body->installEventFilter(this);
  m_body->setParent(container);
  m_body->move(0, 0);
  m_layers->add(container);
  m_scrollable_layer = new ScrollableLayer();
  update_ranges();
  m_scrollable_layer->get_vertical_scroll_bar().connect_position_signal(
    [=] (auto position) { on_vertical_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().connect_position_signal(
    [=] (auto position) { on_horizontal_scroll(position); });
  m_layers->add(m_scrollable_layer);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_layers);
  setMaximumSize(m_body->size());
}

bool ScrollBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    setMaximumSize(m_body->size());
    update_ranges();
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollBox::resizeEvent(QResizeEvent* event) {
  update_ranges();
}

void ScrollBox::on_vertical_scroll(int position) {
  m_body->move(m_body->pos().x(), -position);
}

void ScrollBox::on_horizontal_scroll(int position) {
  m_body->move(-position, m_body->pos().y());
}

void ScrollBox::update_ranges() {
  auto vertical_range = std::max(m_body->height() - height(), 0);
  auto horizontal_range = std::max(m_body->width() - width(), 0);
  m_scrollable_layer->get_vertical_scroll_bar().set_range(0, vertical_range);
  m_scrollable_layer->get_vertical_scroll_bar().set_page_size(height());
  m_scrollable_layer->get_horizontal_scroll_bar().set_range(
    0, horizontal_range);
  m_scrollable_layer->get_horizontal_scroll_bar().set_page_size(width());
}
