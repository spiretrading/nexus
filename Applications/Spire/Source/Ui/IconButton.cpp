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
    : QAbstractButton(parent),
      m_last_focus_reason(Qt::NoFocusReason),
      m_icon(std::move(icon)),
      m_default_color("#7F5EEC"),
      m_hover_color("#4B23A0"),
      m_disabled_color("#D0D0D0"),
      m_blur_color(m_default_color) {
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

void IconButton::set_icon(const QImage& icon) {
  m_icon = icon;
  update();
}

void IconButton::set_blur_color(const QColor& color) {
  m_blur_color = color;
}

void IconButton::set_default_color(const QColor& color) {
  m_default_color = color;
}

void IconButton::set_hover_color(const QColor& color) {
  m_hover_color = color;
}

void IconButton::set_default_background_color(const QColor& color) {
  m_default_background_color = color;
}

void IconButton::set_hover_background_color(const QColor& color) {
  m_hover_background_color = color;
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
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), get_current_icon_color());
  painter.drawPixmap(0, 0, icon);
  if(hasFocus() && is_last_focus_reason_tab()) {
    painter.setPen("#4B23A0");
    auto path = QPainterPath();
    path.addRoundedRect(0, 0, width() -1 , height() - 1, scale_width(2),
      scale_height(2));
    painter.setPen({QColor("#4B23A0"), static_cast<qreal>(scale_width(1))});
    painter.drawPath(path);
  }
}

const QColor& IconButton::get_current_icon_color() const {
  if(isEnabled()) {
    if(underMouse() || (hasFocus() && is_last_focus_reason_tab())) {
      return m_hover_color;
    } else if(!window()->isActiveWindow()) {
      return m_blur_color;
    }
    return m_default_color;
  }
  return m_disabled_color;
}

bool IconButton::is_last_focus_reason_tab() const {
  return m_last_focus_reason == Qt::TabFocusReason ||
    m_last_focus_reason == Qt::BacktabFocusReason;
}
