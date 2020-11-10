#include "Spire/Ui/IconButton.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

IconButton::IconButton(QImage icon, QWidget* parent)
  : IconButton(icon, icon, parent) {}

IconButton::IconButton(QImage icon, QImage hover_icon, QWidget* parent)
  : IconButton(icon, hover_icon, icon, parent) {}

IconButton::IconButton(QImage icon, QImage hover_icon,
  QImage blur_icon, QWidget* parent)
  : IconButton(icon, hover_icon, blur_icon, blur_icon, parent) {}

IconButton::IconButton(QImage icon, QImage hover_icon, QImage blur_icon,
    QImage disabled_icon, QWidget* parent)
    : QAbstractButton(parent),
      m_last_focus_reason(Qt::NoFocusReason),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)),
      m_blur_icon(std::move(blur_icon)),
      m_disabled_icon(std::move(disabled_icon)) {
  m_default_background_color.setAlpha(0);
  m_hover_background_color.setAlpha(0);
  setAttribute(Qt::WA_Hover);
  setMouseTracking(true);
  setStyleSheet(QString(R"(
    QToolTip {
      background-color: white;
      border: 1px solid #C8C8C8;
      color: black;
      font-family: Roboto;
      font-size: %1px;
      padding: %2px %3px %2px %3px;
    })").arg(scale_height(10)).arg(scale_height(2)).arg(scale_width(6)));
}

void IconButton::set_default_background_color(const QColor& color) {
  m_default_background_color = color;
}

void IconButton::set_hover_background_color(const QColor& color) {
  m_hover_background_color = color;
}

const QImage& IconButton::get_icon() const {
  return m_icon;
}

void IconButton::set_icon(QImage icon) {
  set_icon(icon, icon);
}

void IconButton::set_icon(QImage icon, QImage hover_icon) {
  set_icon(icon, hover_icon, icon);
}

void IconButton::set_icon(QImage icon, QImage hover_icon, QImage blur_icon) {
  m_icon = std::move(icon);
  m_hover_icon = std::move(hover_icon);
  m_blur_icon = std::move(blur_icon);
  update();
}

connection IconButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void IconButton::focusInEvent(QFocusEvent* event) {
  m_last_focus_reason = event->reason();
  update();
  QWidget::focusInEvent(event);
}

void IconButton::focusOutEvent(QFocusEvent* event) {
  update();
  QWidget::focusOutEvent(event);
}

void IconButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    m_clicked_signal();
    event->accept();
    return;
  }
  event->ignore();
}

void IconButton::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
  }
}

void IconButton::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(rect().contains(event->localPos().toPoint())) {
      event->accept();
      m_clicked_signal();
    }
  }
}

void IconButton::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  if(!underMouse() || !isEnabled()) {
    painter.fillRect(rect(), m_default_background_color);
  } else {
    painter.fillRect(rect(), m_hover_background_color);
  }
  const auto& current_icon = get_current_icon();
  auto icon_size = current_icon.size();
  auto image_pos = QPoint((width() - icon_size.width()) / 2,
    ((height() - icon_size.height()) / 2) + (width() - height()) / 2);
  painter.drawImage(image_pos, current_icon);
  if(hasFocus() && is_last_focus_reason_tab()) {
    painter.fillRect(image_pos.x(), event->rect().height() - scale_height(2),
      icon_size.width(), scale_height(2), QColor("#4B23A0"));
  }
}

const QImage& IconButton::get_current_icon() const {
  if(isEnabled()) {
    if(underMouse() || (hasFocus() && is_last_focus_reason_tab())) {
      return m_hover_icon;
    } else if(!window()->isActiveWindow()) {
      return m_blur_icon;
    }
    return m_icon;
  }
  return m_disabled_icon;
}

bool IconButton::is_last_focus_reason_tab() const {
  return m_last_focus_reason == Qt::TabFocusReason ||
    m_last_focus_reason == Qt::BacktabFocusReason;
}
