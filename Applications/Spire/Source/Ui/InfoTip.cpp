#include "Spire/Ui/InfoTip.hpp"
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DEFAULT_SHOW_DELAY_MS = 500;
  const auto DROP_SHADOW_SIZE = 5;
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 63);
  const auto FADE_IN_MS = 100;

  auto ARROW_SIZE() {
    static auto size = scale(14, 7);
    return size;
  }

  auto ARROW_X_POSITION() {
    static auto x = scale_width(6);
    return x;
  }

  auto DROP_SHADOW_HEIGHT() {
    static auto height = scale_height(DROP_SHADOW_SIZE);
    return height;
  }

  auto DROP_SHADOW_WIDTH() {
    static auto width = scale_width(DROP_SHADOW_SIZE);
    return width;
  }

  auto Y_OFFSET() {
    static auto offset = scale_height(3);
    return offset;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border_color(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    return style;
  }
}

InfoTip::InfoTip(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_body(body),
      m_is_interactive(false) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(get_margins());
  layout->addWidget(m_body);
  m_show_timer.setInterval(DEFAULT_SHOW_DELAY_MS);
  m_show_timer.setSingleShot(true);
  connect(&m_show_timer, &QTimer::timeout, this, &InfoTip::on_show_timeout);
  parent->installEventFilter(this);
  connect_style_signal(*this, [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
}

bool InfoTip::eventFilter(QObject* watched, QEvent* event) {
  switch(event->type()) {
    case QEvent::Enter:
      if(parentWidget()->isEnabled()) {
        m_show_timer.start();
      }
      break;
    case QEvent::MouseMove:
      if(!parentWidget()->rect().contains(
          static_cast<QMouseEvent*>(event)->pos())) {
        fade_out();
      }
      break;
    case QEvent::Leave:
      if(!(m_is_interactive && hover_rect().contains(QCursor::pos()))) {
        fade_out();
      }
      break;
    case QEvent::WindowDeactivate:
      fade_out();
      break;
    case QEvent::ToolTip:
      return true;
  }
  return QWidget::eventFilter(watched, event);
}

void InfoTip::leaveEvent(QEvent* event) {
  fade_out();
  QWidget::leaveEvent(event);
}

void InfoTip::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.drawPixmap(0, 0, render_background());
}

void InfoTip::set_interactive(bool is_interactive) {
  m_is_interactive = is_interactive;
}

void InfoTip::fade_in() {
  fade_window(this, false, FADE_IN_MS);
}

void InfoTip::fade_out() {
  auto animation = fade_window(this, true, FADE_IN_MS);
  connect(animation, &QPropertyAnimation::finished, [&] {
    if(!parentWidget()->underMouse()) {
      m_show_timer.stop();
      hide();
    }
  });
}

QPainterPath InfoTip::get_arrow_path() const {
  auto path = QPainterPath();
  auto polygon = [&] () -> QPolygonF {
    auto margins = get_margins();
    auto left_x = ARROW_X_POSITION() + DROP_SHADOW_WIDTH();
    auto tip_x = left_x + (ARROW_SIZE().width() / 2);
    auto right_x = left_x + ARROW_SIZE().width();
    auto orientation = get_orientation();
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      return QVector<QPoint>({{left_x, height() - margins.bottom()},
        {tip_x, height() - Y_OFFSET()},
        {right_x, height() - margins.bottom()}});
    }
    return QVector<QPoint>({{left_x, margins.top()}, {tip_x, Y_OFFSET()},
      {right_x, margins.top()}});
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    polygon.translate(width() - (2 * (ARROW_X_POSITION() +
      DROP_SHADOW_WIDTH())) - ARROW_SIZE().width(), 0);
  }
  path.addPolygon(polygon);
  return path;
}

InfoTip::BodyOrientation InfoTip::get_body_orientation() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_geometry =
    get_current_screen(parent_position)->availableGeometry();
  if(parent_position.x() + width() >
      screen_geometry.x() + screen_geometry.width()) {
    return BodyOrientation::LEFT;
  }
  return BodyOrientation::RIGHT;
}

QScreen* InfoTip::get_current_screen(const QPoint& point) const {
  if(auto screen = QGuiApplication::screenAt(point)) {
    return screen;
  }
  return parentWidget()->screen();
}

QMargins InfoTip::get_margins() const {
  auto orientation = get_orientation();
  if(orientation == Orientation::TOP_LEFT ||
      orientation == Orientation::TOP_RIGHT) {
    return {DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT(), DROP_SHADOW_WIDTH(),
      Y_OFFSET() + ARROW_SIZE().height()};
  }
  return {DROP_SHADOW_WIDTH(), Y_OFFSET() + ARROW_SIZE().height(),
    DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT()};
}

InfoTip::Orientation InfoTip::get_orientation() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_geometry =
    get_current_screen(parent_position)->availableGeometry();
  if((parent_position.y() + height()) >
      screen_geometry.y() + screen_geometry.height()) {
    if(parent_position.x() < screen_geometry.x()) {
      return Orientation::TOP_RIGHT;
    }
    return Orientation::TOP_LEFT;
  } else if(parent_position.x() < screen_geometry.x()) {
    return Orientation::BOTTOM_RIGHT;
  }
  return Orientation::BOTTOM_LEFT;
}

QPoint InfoTip::get_position() const {
  auto parent_pos = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto orientation = get_orientation();
  auto x = [&] {
    if(orientation == Orientation::BOTTOM_LEFT ||
        orientation == Orientation::TOP_LEFT) {
      return parent_pos.x() - DROP_SHADOW_WIDTH();
    }
    return parent_pos.x() + (parentWidget()->width() -
      (2 * ARROW_X_POSITION()) - ARROW_SIZE().width() - DROP_SHADOW_WIDTH());
  }();
  auto y = [&] {
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      return parent_pos.y() - parentWidget()->height() - height() -
        scale_height(1);
    }
    return parent_pos.y() + scale_height(1);
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    x -= width() - ((2 * DROP_SHADOW_WIDTH()) + ARROW_SIZE().width() +
      (2 * ARROW_X_POSITION()));
  }
  return {x, y};
}

QRect InfoTip::hover_rect() const {
  return QRect(get_position(), QSize(width(), height()));
}

QPixmap InfoTip::render_background() {
  auto scene = QGraphicsScene();
  scene.setSceneRect(rect());
  auto shadow = QGraphicsDropShadowEffect();
  shadow.setColor(DROP_SHADOW_COLOR);
  shadow.setOffset(0, scale_height(3));
  shadow.setBlurRadius(scale_width(5));
  auto path = get_arrow_path();
  path.addRect(rect().marginsRemoved(get_margins()));
  auto arrow = scene.addPath(path.simplified(), QPen(m_border_color,
    scale_width(1)), m_background_color);
  arrow->setGraphicsEffect(&shadow);
  auto pixmap = QPixmap(size());
  pixmap.fill(Qt::transparent);
  auto painter = QPainter(&pixmap);
  scene.render(&painter);
  return pixmap;
}

void InfoTip::on_show_timeout() {
  if(isEnabled() && parentWidget()->underMouse()) {
    layout()->setContentsMargins(get_margins());
    adjustSize();
    move(get_position());
    fade_in();
    show();
  }
}

void InfoTip::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_background_color = color;
        });
      },
      [&] (const BorderTopColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_border_color = color;
        });
      });
  }
  update();
}
