#include "Spire/Ui/Tooltip.hpp"
#include <QCursor>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace Spire;

namespace {
  const auto DEFAULT_SHOW_DELAY_MS = 500;

  const auto BOTTOM_LEFT_OFFSET() {
    static auto offset = QPoint(0, scale_height(5));
    return offset;
  }

  const auto MOUSE_OFFSET() {
    static auto offset = QPoint(scale_width(10), scale_height(10));
    return offset;
  }

  const auto& QLABEL_TOOLTIP_STYLE() {
    static auto style = QString(R"(
      QLabel {
        background-color: #333333;
        color: #FFFFFF;
        font-family: Roboto;
        font-size: %1px;
        font-weight: 500;
        padding: %2px %3px;
      }
    )").arg(scale_height(10)).arg(scale_width(6)).arg(scale_height(6));
    return style;
  }
}

Tooltip::Tooltip(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_position(Position::BOTTOM_LEFT) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  auto shadow = new DropShadow(true, true, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(body);
  m_show_timer.setInterval(DEFAULT_SHOW_DELAY_MS);
  m_show_timer.setSingleShot(true);
  connect(&m_show_timer, &QTimer::timeout, this, &Tooltip::on_show_timeout);
  parent->installEventFilter(this);
}

bool Tooltip::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parentWidget()) {
    switch(event->type()) {
      case QEvent::HoverEnter:
        if(parentWidget()->isEnabled()) {
          m_show_timer.start();
        }
        break;
      case QEvent::HoverLeave:
      case QEvent::WindowDeactivate:
        m_show_timer.stop();
        hide();
        break;
      case QEvent::ToolTip:
        return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void Tooltip::set_position(Position position) {
  set_position(position, 0, 0);
}

void Tooltip::set_position(Position position, int x, int y) {
  m_position = position;
  m_position_offset = {x, y};
}

QPoint Tooltip::get_position() const {
  auto position = [&] {
    if(m_position == Position::BOTTOM_LEFT) {
      return parentWidget()->mapToGlobal(parentWidget()->rect().bottomLeft()) +
        BOTTOM_LEFT_OFFSET();
    }
    return QCursor::pos() + MOUSE_OFFSET();
  }();
  return position + m_position_offset;
}

void Tooltip::on_show_timeout() {
  if(parentWidget()->underMouse()) {
    move(get_position());
    show();
  }
}

Tooltip* Spire::make_text_tooltip(const QString& label, QWidget* parent) {
  auto body = new QLabel(label, parent);
  body->setStyleSheet(QLABEL_TOOLTIP_STYLE());
  auto tooltip = new Tooltip(body, parent);
  return tooltip;
}
