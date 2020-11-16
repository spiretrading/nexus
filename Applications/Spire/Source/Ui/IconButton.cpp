#include "Spire/Ui/IconButton.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

IconButton::Style::Style()
    : m_default_color("#7F5EEC"),
      m_hover_color("#4B23A0"),
      m_disabled_color("#D0D0D0"),
      m_hover_background_color("#E3E3E3"),
      m_blur_color(m_default_color) {
  m_default_background_color.setAlpha(0);
}

IconButton::IconButton(QImage icon, QWidget* parent)
    : QAbstractButton(parent),
      m_last_focus_reason(Qt::NoFocusReason),
      m_icon(std::move(icon)) {
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

const IconButton::Style& IconButton::get_style() const {
  return m_style;
}

void IconButton::set_style(const Style& style) {
  m_style = style;
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
    painter.fillRect(rect(), m_style.m_default_background_color);
  } else {
    painter.fillRect(rect(), m_style.m_hover_background_color);
  }
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), get_current_icon_color());
  painter.drawPixmap(0, 0, icon);
  if(hasFocus() && is_last_focus_reason_tab()) {
    painter.setPen("#4B23A0");
    auto path = QPainterPath();
    path.addRoundedRect(0, 0, width() -1 , height() - 1, scale_width(1),
      scale_height(1));
    painter.setPen({QColor("#4B23A0"), static_cast<qreal>(scale_width(1))});
    painter.drawPath(path);
  }
}

const QColor& IconButton::get_current_icon_color() const {
  if(isEnabled()) {
    if(underMouse() || (hasFocus() && is_last_focus_reason_tab())) {
      return m_style.m_hover_color;
    } else if(!window()->isActiveWindow()) {
      return m_style.m_blur_color;
    }
    return m_style.m_default_color;
  }
  return m_style.m_disabled_color;
}

bool IconButton::is_last_focus_reason_tab() const {
  return m_last_focus_reason == Qt::TabFocusReason ||
    m_last_focus_reason == Qt::BacktabFocusReason;
}
