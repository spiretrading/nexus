#include "spire/ui/window.hpp"
#include <QColor>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QWindowStateChangeEvent>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

namespace {
  auto make_drop_shadow_effect(window* w) {
    auto drop_shadow = new QGraphicsDropShadowEffect(w);
    drop_shadow->setBlurRadius(scale_width(12));
    drop_shadow->setXOffset(0);
    drop_shadow->setYOffset(0);
    drop_shadow->setColor(QColor(0, 0, 0, 100));
    return drop_shadow;
  }
}

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  this->::QWidget::window()->setWindowFlags(
    this->::QWidget::window()->windowFlags() | Qt::Window |
    Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  this->::QWidget::window()->setAttribute(Qt::WA_TranslucentBackground);
  resize(m_body->width() + scale_width(24),
    m_body->height() + scale_height(24));
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  auto drop_shadow = make_drop_shadow_effect(this);
  layout->setMargin(drop_shadow->blurRadius());
  setGraphicsEffect(drop_shadow);
  m_title_bar = new title_bar(this);
  layout->addWidget(m_title_bar);
  layout->addWidget(m_body);
  m_title_bar->connect_maximize_signal([=] { on_maximize(); });
  installEventFilter(m_title_bar);
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
    auto& changeEvent = *static_cast<QWindowStateChangeEvent*>(event);
    if(!windowState().testFlag(Qt::WindowMinimized) &&
        graphicsEffect() == nullptr) {
      auto drop_shadow = make_drop_shadow_effect(this);
      layout()->setMargin(drop_shadow->blurRadius());
      setGraphicsEffect(drop_shadow);
    }
    if(windowState().testFlag(Qt::WindowMinimized)) {
      setGraphicsEffect(nullptr);
      layout()->setMargin(0);
    } else if(windowState().testFlag(Qt::WindowMaximized)) {
      layout()->setMargin(0);
    } else if(layout()->margin() == 0) {
      auto effect = static_cast<const QGraphicsDropShadowEffect*>(
        graphicsEffect());
      layout()->setMargin(effect->blurRadius());
    }
  }
  QWidget::changeEvent(event);
}

void window::on_maximize() {
  setGraphicsEffect(nullptr);
  layout()->setMargin(0);
  repaint();
}
