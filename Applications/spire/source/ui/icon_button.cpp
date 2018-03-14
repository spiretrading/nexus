#include "spire/ui/icon_button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSize>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

icon_button::icon_button(const QImage& default_icon, const QImage& hover_icon,
    QWidget* parent)
    : QWidget(parent),
      m_clickable(true) {
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  m_label = new QLabel(this);
  m_default_icon = default_icon;
  m_hover_icon = hover_icon;
  m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  layout->addWidget(m_label);
}

connection icon_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void icon_button::enterEvent(QEvent* event) {
  m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
}

void icon_button::leaveEvent(QEvent* event) {
  m_label->setPixmap(QPixmap::fromImage(m_default_icon));
}

void icon_button::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void icon_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && m_clickable) {
    if(rect().contains(event->localPos().toPoint())) {
      m_clicked_signal();
    }
  }
}

void icon_button::set_clickable(bool clickable) {
  m_clickable = clickable;
}

void icon_button::swap_icons() {
  if(m_is_default) {
    m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  } else {
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
  m_is_default = !m_is_default;
}

void icon_button::set_icon(bool is_default) {
  if(is_default) {
    m_label->setPixmap(QPixmap::fromImage(m_default_icon));
  } else {
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
  m_is_default = is_default;
}
