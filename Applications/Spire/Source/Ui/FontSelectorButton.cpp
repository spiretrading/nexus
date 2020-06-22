#include "Spire/Ui/FontSelectorButton.hpp"
#include <QKeyEvent>
#include <QPainter>

using namespace boost::signals2;
using namespace Spire;

FontSelectorButton::FontSelectorButton(QImage icon, QImage toggle_icon,
    QImage hover_icon, QWidget* parent)
    : QWidget(parent),
      m_icon(std::move(icon)),
      m_toggle_icon(std::move(toggle_icon)),
      m_hover_icon(std::move(hover_icon)),
      m_is_toggled(false) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
}

connection FontSelectorButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void FontSelectorButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    swap_toggle();
    m_clicked_signal();
    update();
  }
}

void FontSelectorButton::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    swap_toggle();
    m_clicked_signal();
    update();
  }
}

void FontSelectorButton::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_is_toggled) {
    painter.drawImage(0, 0, m_toggle_icon);
    if(underMouse() || hasFocus()) {
      painter.setPen(QColor("#4B23A0"));
      painter.drawRect(0, 0, width() - 1, height() - 1);
    }
  } else if(underMouse() || hasFocus()) {
    painter.drawImage(0, 0, m_hover_icon);
  } else {
    painter.drawImage(0, 0, m_icon);
  }
}

void FontSelectorButton::swap_toggle() {
  m_is_toggled = !m_is_toggled;
}
