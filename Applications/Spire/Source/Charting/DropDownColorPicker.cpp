#include "Spire/Charting/DropDownColorPicker.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Charting/ColorPicker.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::signals2;
using namespace Spire;

DropDownColorPicker::DropDownColorPicker(QWidget* parent)
    : QWidget(parent),
      m_has_mouse(false) {
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  window()->installEventFilter(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_current_color = QColor("#FFCA19");
  m_stored_color = m_current_color;
  m_color_picker = new ColorPicker(scale_width(70) - 4, scale_height(70) - 3,
    this);
  m_color_picker->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  m_color_picker->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_color_picker->connect_selected_signal(
    [=] (auto& color) { on_color_selected(color); });
  m_color_picker->connect_preview_signal(
    [=] (auto& color) { on_color_preview(color); });
  m_color_picker->setFixedSize(scale(70, 70));
  m_color_picker->hide();
  m_color_picker_drop_shadow = new DropShadow(true, true, m_color_picker);
}

connection DropDownColorPicker::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void DropDownColorPicker::enterEvent(QEvent* event) {
  m_has_mouse = true;
  update();
}

bool DropDownColorPicker::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move || event->type() == QEvent::Resize) {
      move_color_picker();
    } else if(event->type() == QEvent::MouseButtonPress) {
      m_color_picker->hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownColorPicker::focusOutEvent(QFocusEvent* event) {
  if(!m_color_picker->isActiveWindow()) {
    m_current_color = m_stored_color;
    m_color_picker->hide();
  }
  update();
}

void DropDownColorPicker::leaveEvent(QEvent* event) {
  m_has_mouse = false;
  update();
}

void DropDownColorPicker::mousePressEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    if(m_color_picker->isVisible()) {
      m_color_picker->hide();
    } else {
      m_color_picker->show();
    }
  }
}

void DropDownColorPicker::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_color_picker->hasFocus() || m_has_mouse) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, m_current_color);
}

void DropDownColorPicker::showEvent(QShowEvent* event) {
  move_color_picker();
}

void DropDownColorPicker::on_color_selected(const QColor& color) {
  m_current_color = color;
  m_stored_color = color;
  m_color_picker->hide();
  m_has_mouse = false;
  update();
  m_color_signal(color);
}

void DropDownColorPicker::on_color_preview(const QColor& color) {
  m_current_color = color;
  update();
}

void DropDownColorPicker::move_color_picker() {
  auto pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft());
  m_color_picker->move(pos.x(), pos.y() + scale_height(1));
  m_color_picker->raise();
}
