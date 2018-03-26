#include "spire/ui/icon_button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>

using namespace boost;
using namespace boost::signals2;
using namespace spire;

icon_button::icon_button(QImage icon, QWidget* parent)
    : icon_button(icon, icon, parent) {}

icon_button::icon_button(QImage icon, QImage hover_icon, QWidget* parent)
    : QWidget(parent),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  m_label = new QLabel(this);
  m_label->setPixmap(QPixmap::fromImage(m_icon));
  layout->addWidget(m_label);
}

void icon_button::set_default_style(const QString& stylesheet) {
  m_default_stylesheet = stylesheet;
}

void icon_button::set_hover_style(const QString& stylesheet) {
  m_hover_stylesheet = stylesheet;
}

const QImage& icon_button::get_icon() const {
  return m_icon;
}

void icon_button::set_icon(QImage icon) {
  set_icon(icon, icon);
}

void icon_button::set_icon(QImage icon, QImage hover_icon) {
  m_icon = std::move(icon);
  m_hover_icon = std::move(hover_icon);
  if(hasFocus()) {
    setStyleSheet(m_hover_stylesheet);
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  } else {
    setStyleSheet(m_default_stylesheet);
    m_label->setPixmap(QPixmap::fromImage(m_icon));
  }
}

connection icon_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void icon_button::enterEvent(QEvent* event) {
  if(isEnabled()) {
    setStyleSheet(m_hover_stylesheet);
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
}

void icon_button::focusInEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    setStyleSheet(m_hover_stylesheet);
    m_label->setPixmap(QPixmap::fromImage(m_hover_icon));
  }
}

void icon_button::focusOutEvent(QFocusEvent* event) {
  if(focusPolicy() & Qt::TabFocus) {
    setStyleSheet(m_default_stylesheet);
    m_label->setPixmap(QPixmap::fromImage(m_icon));
  }
}

void icon_button::leaveEvent(QEvent* event) {
  setStyleSheet(m_default_stylesheet);
  m_label->setPixmap(QPixmap::fromImage(m_icon));
}

void icon_button::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
  }
}

void icon_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(rect().contains(event->localPos().toPoint())) {
      event->accept();
      m_clicked_signal();
    }
  }
}
