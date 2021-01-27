#include "Spire/Ui/Tooltip.hpp"
#include <QCursor>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto DEFAULT_SHOW_DELAY_MS = 500;
  const auto DROP_SHADOW_SIZE = 3;
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 89);
  const auto TOOLTIP_COLOR = QColor("#333333");

  const auto ARROW_X_POSITION() {
    static auto x = scale_width(8);
    return x;
  }

  const auto ARROW_SIZE() {
    static auto size = scale(10, 5);
    return size;
  }

  const auto Y_OFFSET() {
    static auto offset = scale_height(3);
    return offset;
  }

  const auto& QLABEL_TOOLTIP_STYLE() {
    static auto style = QString(R"(
      QLabel {
        background-color: transparent;
        color: #FFFFFF;
        font-family: Roboto;
        font-size: %1px;
        font-weight: 600;
        padding: %2px %3px;
      }
    )").arg(scale_height(10)).arg(scale_width(6)).arg(scale_height(6));
    return style;
  }
}

Tooltip::Tooltip(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_body(body) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_TranslucentBackground);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(body);
  m_show_timer.setInterval(DEFAULT_SHOW_DELAY_MS);
  m_show_timer.setSingleShot(true);
  connect(&m_show_timer, &QTimer::timeout, this, &Tooltip::on_show_timeout);
  m_body->installEventFilter(this);
  parent->installEventFilter(this);
  auto shadow = new QGraphicsDropShadowEffect(this);
  shadow->setColor(DROP_SHADOW_COLOR);
  shadow->setOffset(0, 0);
  shadow->setBlurRadius(scale_width(5));
  setGraphicsEffect(shadow);
}

bool Tooltip::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::Resize) {
      resize(m_body->size().grownBy(get_margins()));
    }
  } else if(watched == parentWidget()) {
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

void Tooltip::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  auto body_rect = rect().marginsRemoved(get_margins());
  painter.fillRect(body_rect, TOOLTIP_COLOR);
  painter.fillPath(get_arrow_path(), TOOLTIP_COLOR);
}

void Tooltip::showEvent(QShowEvent* event) {
  layout()->setContentsMargins(get_margins());
  QWidget::showEvent(event);
}

QPainterPath Tooltip::get_arrow_path() const {
  auto path = QPainterPath();
  auto polygon = [&] () -> QPolygonF {
    auto margins = get_margins();
    auto x = ARROW_X_POSITION() + scale_width(DROP_SHADOW_SIZE);
    if(get_orientation() == Orientation::TOP) {
      return QVector<QPoint>({{x, height() - margins.bottom()},
        {x + (ARROW_SIZE().width() / 2), height() - Y_OFFSET()},
        {x + ARROW_SIZE().width(), height() - margins.bottom()}});
    }
    return QVector<QPoint>({{x, margins.top()},
      {x + (ARROW_SIZE().width() / 2), Y_OFFSET()},
      {x + ARROW_SIZE().width(), margins.top()}});
  }();
  path.addPolygon(polygon);
  return path;
}

QMargins Tooltip::get_margins() const {
  if(get_orientation() == Orientation::TOP) {
    return {scale_width(DROP_SHADOW_SIZE), scale_height(DROP_SHADOW_SIZE),
      scale_width(DROP_SHADOW_SIZE), Y_OFFSET() + ARROW_SIZE().height()};
  }
  return {scale_width(DROP_SHADOW_SIZE), Y_OFFSET() + ARROW_SIZE().height(),
    scale_width(DROP_SHADOW_SIZE), scale_height(DROP_SHADOW_SIZE)};
}

Tooltip::Orientation Tooltip::get_orientation() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_height = screen()->availableGeometry().height();
  if((parent_position.y() + height()) > screen_height) {
    return Orientation::TOP;
  }
  return Orientation::BOTTOM;
}

QPoint Tooltip::get_position() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_height = screen()->size().height();
  if(get_orientation() == Orientation::TOP) {
    auto top_left = parentWidget()->mapToGlobal(
      parentWidget()->rect().topLeft());
    return top_left - QPoint(scale_width(DROP_SHADOW_SIZE),
      height() + scale_height(1));
  }
  return parent_position + QPoint(-scale_width(DROP_SHADOW_SIZE),
    scale_height(1));
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
