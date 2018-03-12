#include "spire/ui/window.hpp"
#include <QColor>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace spire;

window::window(QWidget* widget, QWidget* parent)
    : QWidget(parent),
    m_window(widget) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  this->setFixedSize(widget->width() + scale_width(24),
    widget->height() + scale_height(24));
  auto outer_layout = new QHBoxLayout(this);
  outer_layout->setSpacing(0);
  m_border_widget = new QWidget(this);
  m_border_widget->setFixedSize(widget->width() + 2, widget->height() + 2);
  outer_layout->addWidget(m_border_widget);
  auto inner_layout = new QHBoxLayout(m_border_widget);
  inner_layout->setMargin(0);
  inner_layout->setSpacing(0);
  inner_layout->addWidget(widget);
  widget->setParent(m_border_widget);
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(25);
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  drop_shadow->setColor(QColor(0, 0, 0, 70));
  m_border_widget->setGraphicsEffect(drop_shadow);
  auto ss = widget->styleSheet();
  set_stylesheet("#C8C8C8");
  widget->setGraphicsEffect(nullptr);
  widget->setStyleSheet(ss + "border: 0px solid white;");
  show();
}

window::window(QWidget* widget, const QString& color_hex,
    QWidget* parent)
    : window(widget, parent) {
  set_stylesheet(color_hex);
}

QWidget* window::get_window() {
  return m_window;
}

void window::set_stylesheet(const QString& color_hex) {
  m_border_widget->setStyleSheet(QString(
    "border: 1px solid %1;").arg(color_hex));
}
