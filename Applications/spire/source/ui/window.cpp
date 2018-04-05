#include "spire/ui/window.hpp"
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
  resize(m_body->width() + scale_width(25),
    m_body->height() + scale_height(25));
  auto drop_shadow = make_drop_shadow_effect(this);
  setGraphicsEffect(drop_shadow);
  auto layout = new QVBoxLayout(this);
  layout->setMargin(drop_shadow->blurRadius());
  layout->setSpacing(0);
  m_border = new QWidget(this);
  m_border->setObjectName("window_border");
  m_border->resize(m_body->size() + scale(1, 1));
  set_border_stylesheet("#A0A0A0");
  layout->addWidget(m_border);
  auto border_layout = new QVBoxLayout(m_border);
  border_layout->setMargin(scale_width(1));
  border_layout->setSpacing(0);
  m_title_bar = new title_bar(m_border);
  border_layout->addWidget(m_title_bar);
  border_layout->addWidget(m_body);
  m_title_bar->connect_maximize_signal([=] { on_maximize(); });
  this->::QWidget::window()->installEventFilter(this);
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
      layout()->setContentsMargins({});
    } else if(windowState().testFlag(Qt::WindowMaximized)) {
      layout()->setContentsMargins({});
    } else if(layout()->margin() == 0) {
      auto effect = static_cast<const QGraphicsDropShadowEffect*>(
        graphicsEffect());
      layout()->setMargin(effect->blurRadius());
    }
  }
  QWidget::changeEvent(event);
}

bool window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == this->::QWidget::window()) {
    if(event->type() == QEvent::WindowActivate) {
      set_border_stylesheet("#A0A0A0");
    } else if(event->type() == QEvent::WindowDeactivate) {
      set_border_stylesheet("#C8C8C8");
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::set_border_stylesheet(const QColor& color) {
  m_border->setStyleSheet(QString(R"(
    #window_border {
                    border: %1px solid %3 %2px solid %3;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(color.name()));
}

void window::on_maximize() {
  setGraphicsEffect(nullptr);
  layout()->setContentsMargins({});
  repaint();
}
