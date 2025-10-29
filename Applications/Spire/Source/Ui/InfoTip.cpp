#include "Spire/Ui/InfoTip.hpp"
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DROP_SHADOW_SIZE = 5;
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
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xA0A0A0)));
    return style;
  }
}

InfoTip::InfoTip(QWidget* body, QWidget* parent)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::ToolTip |
        Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus),
      m_parent(parent),
      m_is_interactive(false),
      m_hover_observer(*parent),
      m_fade_state(FadeState::NONE),
      m_animation(nullptr) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_container = new QWidget(this);
  auto layout = make_hbox_layout(this);
  layout->setContentsMargins(get_margins());
  layout->addWidget(m_container);
  auto container_layout = make_hbox_layout(m_container);
  container_layout->addWidget(body);
  const auto DEFAULT_SHOW_DELAY_MS = 500;
  m_show_timer.setInterval(DEFAULT_SHOW_DELAY_MS);
  m_show_timer.setSingleShot(true);
  connect(&m_show_timer, &QTimer::timeout, this, &InfoTip::on_show_timeout);
  const auto INTERACTIVE_PERIOD_MS = 300;
  m_interactive_timer.setInterval(INTERACTIVE_PERIOD_MS);
  connect(&m_interactive_timer, &QTimer::timeout, this,
    &InfoTip::on_interactive_timeout);
  parent->installEventFilter(this);
  match(*body, Body());
  link(*this, *body);
  m_hover_observer.connect_state_signal(
    std::bind_front(&InfoTip::on_hover, this));
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&InfoTip::on_style, this));
  set_style(*this, DEFAULT_STYLE());
  connect(body, &QObject::destroyed, this, [=] {
    deleteLater();
  });
}

bool InfoTip::eventFilter(QObject* watched, QEvent* event) {
  switch(event->type()) {
    case QEvent::ToolTip:
      return true;
    case QEvent::Destroy:
      deleteLater();
      break;
  }
  return QWidget::eventFilter(watched, event);
}

void InfoTip::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    if(is_hovered()) {
      m_show_timer.start();
    }
  }
  return QWidget::changeEvent(event);
}

void InfoTip::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.drawPixmap(0, 0, render_background());
}

void InfoTip::set_interactive(bool is_interactive) {
  m_is_interactive = is_interactive;
}

QRect InfoTip::get_interactive_region() const {
  return QRect(get_position(), QSize(width(), height()));
}

bool InfoTip::is_hovered() const {
  return isEnabled() && (m_hover_observer.get_state() !=
    HoverObserver::State::NONE || m_is_interactive &&
      get_interactive_region().contains(QCursor::pos()));
}

void InfoTip::fade_in() {
  if(m_fade_state == FadeState::FADING_IN) {
    return;
  }
  m_fade_state = FadeState::FADING_IN;
  if(m_animation) {
    m_animation->stop();
    delete_later(m_animation);
  }
  m_interactive_timer.start();
  m_animation = fade_window(this, false, milliseconds(FADE_IN_MS));
  connect(m_animation, &QPropertyAnimation::finished, this,
    &InfoTip::on_fade_in_finished);
}

void InfoTip::fade_out() {
  if(m_fade_state == FadeState::FADING_OUT) {
    return;
  }
  m_fade_state = FadeState::FADING_OUT;
  if(m_animation) {
    m_animation->stop();
    delete_later(m_animation);
  }
  m_interactive_timer.stop();
  m_animation = fade_window(this, true, milliseconds(FADE_IN_MS));
  connect(m_animation, &QPropertyAnimation::finished, this,
    &InfoTip::on_fade_out_finished);
}

QPainterPath InfoTip::get_arrow_path() const {
  auto path = QPainterPath();
  auto polygon = [&] () -> QPolygon {
    auto margins = get_margins();
    auto left_x = ARROW_X_POSITION() + DROP_SHADOW_WIDTH();
    auto tip_x = left_x + ARROW_SIZE().width() / 2;
    auto right_x = left_x + ARROW_SIZE().width();
    auto orientation = get_orientation();
    auto half_border_size = m_border_size / 2.0;
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      auto y = height() - margins.bottom() -
        static_cast<int>(std::ceil(half_border_size));
      return QVector<QPoint>({{left_x, y}, {tip_x, height() - Y_OFFSET()},
        {right_x, y}});
    }
    auto y = margins.top() + static_cast<int>(half_border_size);
    return QVector<QPoint>({{left_x, y}, {tip_x, Y_OFFSET()}, {right_x, y}});
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    polygon.translate(width() - 2 * (ARROW_X_POSITION() +
      DROP_SHADOW_WIDTH()) - ARROW_SIZE().width(), 0);
  }
  path.addPolygon(polygon);
  return path;
}

InfoTip::BodyOrientation InfoTip::get_body_orientation() const {
  auto parent_position = m_parent->mapToGlobal(m_parent->rect().bottomLeft());
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
  return m_parent->screen();
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
  auto parent_position = m_parent->mapToGlobal(m_parent->rect().bottomLeft());
  auto screen_geometry =
    get_current_screen(parent_position)->availableGeometry();
  if(parent_position.y() + height() >
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
  auto parent_position = m_parent->mapToGlobal(m_parent->rect().bottomLeft());
  auto orientation = get_orientation();
  auto x = [&] {
    if(orientation == Orientation::BOTTOM_LEFT ||
        orientation == Orientation::TOP_LEFT) {
      return parent_position.x() - DROP_SHADOW_WIDTH();
    }
    return parent_position.x() + m_parent->width() -
      2 * ARROW_X_POSITION() - ARROW_SIZE().width() - DROP_SHADOW_WIDTH();
  }();
  auto y = [&] {
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      return parent_position.y() - m_parent->height() - height() -
        scale_height(1);
    }
    return parent_position.y() + scale_height(1);
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    x -= width() - (2 * DROP_SHADOW_WIDTH() + ARROW_SIZE().width() +
      2 * ARROW_X_POSITION());
  }
  return QPoint(x, y);
}

QPixmap InfoTip::render_background() const {
  auto scene = QGraphicsScene();
  scene.setSceneRect(rect());
  auto shadow = QGraphicsDropShadowEffect();
  shadow.setColor(QColor(0, 0, 0, 63));
  shadow.setOffset(0, 0);
  shadow.setBlurRadius(scale_width(5));
  auto path = get_arrow_path();
  auto half_border_size = m_border_size / 2.0;
  auto border_margins = QMargins(half_border_size, half_border_size,
    std::ceil(half_border_size), std::ceil(half_border_size));
  path.addRect(rect().marginsRemoved(get_margins() + border_margins));
  auto arrow = scene.addPath(path.simplified(), QPen(m_border_color,
    m_border_size), m_background_color);
  arrow->setGraphicsEffect(&shadow);
  auto pixmap = QPixmap(size());
  pixmap.fill(Qt::transparent);
  auto painter = QPainter(&pixmap);
  scene.render(&painter);
  return pixmap;
}

void InfoTip::on_fade_in_finished() {
  delete_later(m_animation);
  if(!is_hovered()) {
    fade_out();
  }
}

void InfoTip::on_fade_out_finished() {
  delete_later(m_animation);
  m_show_timer.stop();
  hide();
  if(is_hovered()) {
    m_show_timer.start();
  }
}

void InfoTip::on_hover(HoverObserver::State state) {
  if(is_hovered()) {
    m_show_timer.start();
  } else {
    fade_out();
  }
}

void InfoTip::on_show_timeout() {
  if(is_hovered()) {
    layout()->setContentsMargins(get_margins());
    move(get_position());
    fade_in();
    show();
    adjustSize();
  }
}

void InfoTip::on_interactive_timeout() {
  if(!is_hovered()) {
    fade_out();
  }
}

void InfoTip::on_style() {
  auto padding = std::make_shared<QMargins>();
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
      },
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_border_size = size;
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          padding->setRight(size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          padding->setLeft(size);
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          padding->setTop(size);
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          padding->setBottom(size);
        });
      });
  }
  if(m_border_size == 0) {
    m_border_color = m_background_color;
  }
  m_container->layout()->setContentsMargins(*padding + m_border_size);
  update();
}
