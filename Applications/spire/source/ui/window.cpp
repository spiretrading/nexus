#include "spire/ui/window.hpp"
#include <QColor>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint |
        Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint),
      m_body(body) {
  setAttribute(Qt::WA_TranslucentBackground);
  resize(m_body->width() + scale_width(24),
    m_body->height() + scale_height(24));
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(scale_width(12));
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  drop_shadow->setColor(QColor(255, 0, 0));
  setGraphicsEffect(drop_shadow);
  auto layout = new QVBoxLayout(this);
  layout->setMargin(drop_shadow->blurRadius());
  layout->setSpacing(0);
  m_title_bar = new title_bar(this);
  layout->addWidget(m_title_bar);
  layout->addWidget(m_body);
}

QWidget* window::get_body() {
  return m_body;
}

void window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

void window::changeEvent(QEvent* event) {
  if(event->type() == QEvent::WindowStateChange) {
    if(windowState().testFlag(Qt::WindowMaximized)) {
      layout()->setMargin(0);
    } else if(layout()->margin() == 0) {
      auto effect = static_cast<const QGraphicsDropShadowEffect*>(
        graphicsEffect());
      layout()->setMargin(effect->blurRadius());
    }
  }
  QWidget::changeEvent(event);
}
