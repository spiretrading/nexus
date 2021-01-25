#include "Spire/Ui/Tooltip.hpp"
#include <QCursor>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto DEFAULT_SHOW_DELAY_MS = 500;

  const auto BOTTOM_LEFT_OFFSET() {
    static auto offset = QPoint(0, scale_height(3));
    return offset;
  }

  const auto& QLABEL_TOOLTIP_STYLE() {
    static auto style = QString(R"(
      QLabel {
        background-color: #FFFFFF;
        border: %1px solid #C8C8C8;
        color: #000000;
        font-size: %2px;
        padding: %3px %4px;
      }
    )").arg(scale_width(1)).arg(scale_height(10)).arg(scale_width(4))
        .arg(scale_height(6));
    return style;
  }
}

Tooltip::Tooltip(QWidget* body, QWidget* parent)
      // TODO: better flag than Tool?
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::NoDropShadowWindowHint),
      m_position(Position::BOTTOM_LEFT) {
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
        m_show_timer.start();
        break;
      case QEvent::FocusIn:
      case QEvent::FocusOut:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::Wheel:
      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate:
        m_show_timer.stop();
        hide();
        break;
      case QEvent::HoverLeave:
        // TODO: still within boundaries? else, hide.
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
    return QCursor::pos();
  }();
  return position + m_position_offset;
}

void Tooltip::on_show_timeout() {
  move(get_position());
  show();
}

Tooltip* Spire::make_text_tooltip(const QString& label, QWidget* parent) {
  auto body = new QLabel(label, parent);
  body->setStyleSheet(QLABEL_TOOLTIP_STYLE());
  auto tooltip = new Tooltip(body, parent);
  return tooltip;
}
