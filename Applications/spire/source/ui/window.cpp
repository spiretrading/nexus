#include "spire/ui/window.hpp"
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <windows.h>
#include "spire/spire/dimensions.hpp"

using namespace spire;

window::window(QWidget* w, QWidget* parent)
    : window(w, QColor("#A0A0A0"), parent) {}

window::window(QWidget* w, const QColor& border_color, QWidget* parent)
    : QWidget(parent),
      m_window(w) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(m_window->width() + scale_width(24),
    m_window->height() + scale_height(24));
  auto outer_layout = new QHBoxLayout(this);
  outer_layout->setSpacing(0);
  m_border_widget = new QWidget(this);
  m_border_widget->setFixedSize(m_window->width() + 2, m_window->height() + 2);
  outer_layout->addWidget(m_border_widget);
  auto inner_layout = new QHBoxLayout(m_border_widget);
  inner_layout->setMargin(0);
  inner_layout->setSpacing(0);
  inner_layout->addWidget(m_window);
  m_window->setParent(m_border_widget);
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(scale_height(12));
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  drop_shadow->setColor(QColor(0, 0, 0, 100));
  m_border_widget->setGraphicsEffect(drop_shadow);
  auto ss = m_window->styleSheet();
  set_border_color(border_color);
  m_window->setGraphicsEffect(nullptr);
  m_window->setStyleSheet(ss + "border: 0px solid white;");
}

QWidget* window::get_window() {
  return m_window;
}

void window::set_border_color(const QColor& color) {
  m_border_widget->setStyleSheet(QString(
    "border: 1px solid %1;").arg(color.name()));
}
