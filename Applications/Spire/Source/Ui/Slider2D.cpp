#include "Spire/Ui/Slider2D.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using ImageTrack = StateSelector<void, struct ImageTrackSelectorTag>;
  using ImageThumb = StateSelector<void, struct ImageThumbSelectorTag>;

  auto DEFAULT_THUMB_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > TrackPad()).
      set(BackgroundColor(QColor(0xC8C8C8)));
    style.get(Hover() > TrackPad()).
      set(BackgroundColor(QColor(0xA0A0A0)));
    style.get(Disabled() > TrackPad()).
      set(BackgroundColor(QColor(0xEBEBEB)));
    style.get(Any() > Track()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
    style.get(Any() > ImageTrack()).set(padding(0));
    style.get(Any() > Thumb()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border(scale_width(2), QColor(0x8D78EC))).
      set(border_radius(scale_width(2)));
    style.get(Focus() > Thumb()).
      set(BackgroundColor(QColor(0x7E71B8))).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > (Thumb() && Hover())).
      set(BackgroundColor(QColor(0x8D78EC))).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > (Thumb() && Press())).
      set(BackgroundColor(QColor(0x7E71B8))).
      set(border_color(QColor(0x4B23A0)));
    style.get(Disabled() > Thumb()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(border_color(QColor(0xA0A0A0)));
    style.get(Any() > ImageThumb()).
      set(BackgroundColor(Qt::transparent)).
      set(border_size(0));
    return style;
  }

  auto make_modifiers(const ScalarValueModel<int>& x,
      const ScalarValueModel<int>& y) {
    auto modifiers = QHash<Qt::KeyboardModifier, QPoint>();
    modifiers[Qt::NoModifier] = {x.get_increment(), y.get_increment()};
    return modifiers;
  }

  auto get_range(const ScalarValueModel<int>& model) {
    return static_cast<double>(*model.get_maximum()) - *model.get_minimum();
  }

  auto get_value(double value, const ScalarValueModel<int>& model) {
    return static_cast<int>(std::round(std::clamp(value,
      static_cast<double>(*model.get_minimum()),
        static_cast<double>(*model.get_maximum()))));
  }
}

Slider2D::Slider2D(QWidget* parent)
  : Slider2D(std::make_shared<LocalScalarValueModel<int>>(),
      std::make_shared<LocalScalarValueModel<int>>(), parent) {}

Slider2D::Slider2D(QHash<Qt::KeyboardModifier, QPoint> modifiers,
  QWidget* parent)
  : Slider2D(std::make_shared<LocalScalarValueModel<int>>(),
      std::make_shared<LocalScalarValueModel<int>>(),
        std::move(modifiers), parent) {}

Slider2D::Slider2D(std::shared_ptr<ScalarValueModel<int>> current_x,
  std::shared_ptr<ScalarValueModel<int>> current_y, QWidget* parent)
  : Slider2D(std::move(current_x), std::move(current_y),
      make_modifiers(*current_x, *current_y), parent) {}

Slider2D::Slider2D(std::shared_ptr<ScalarValueModel<int>> current_x,
    std::shared_ptr<ScalarValueModel<int>> current_y,
    QHash<Qt::KeyboardModifier, QPoint> modifiers, QWidget* parent)
    : QWidget(parent),
      m_current_x(std::move(current_x)),
      m_current_y(std::move(current_y)),
      m_modifiers(std::move(modifiers)),
      m_submission({m_current_x->get(), m_current_y->get()}),
      m_focus_observer(*this),
      m_is_mouse_down(false),
      m_is_modified(false) {
  setFocusPolicy(Qt::StrongFocus);
  auto body = new QWidget();
  m_track_image_container = new QLabel();
  m_track_image_container->setScaledContents(true);
  m_track_image_container->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto track_pad = new Box(m_track_image_container);
  track_pad->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*track_pad, TrackPad());
  m_track = new Box(track_pad);
  m_track->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*m_track, Track());
  enclose(*body, *m_track);
  auto thumb_icon = new Icon(QImage());
  m_thumb = new Box(thumb_icon, m_track);
  m_thumb->setFixedSize(DEFAULT_THUMB_SIZE());
  match(*m_thumb, Thumb());
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  m_focus_observer.connect_state_signal(
    std::bind_front(&Slider2D::on_focus, this));
  m_current_x_connection = m_current_x->connect_update_signal(
    std::bind_front(&Slider2D::on_current_x, this));
  m_current_y_connection = m_current_y->connect_update_signal(
    std::bind_front(&Slider2D::on_current_y, this));
  m_track_style_connection = connect_style_signal(*m_track,
    std::bind_front(&Slider2D::on_track_style, this));
  m_thumb_icon_style_connection = connect_style_signal(*thumb_icon,
    std::bind_front(&Slider2D::on_thumb_icon_style, this));
}

const std::shared_ptr<ScalarValueModel<int>>& Slider2D::get_current_x() const {
  return m_current_x;
}

const std::shared_ptr<ScalarValueModel<int>>& Slider2D::get_current_y() const {
  return m_current_y;
}

QPoint Slider2D::get_step_size() const {
  return m_step_size;
}

void Slider2D::set_step_size(const QPoint& step_size) {
  m_step_size = step_size;
}

connection Slider2D::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void Slider2D::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Right) {
    set_current_x(static_cast<double>(m_current_x->get()) +
      get_increment(static_cast<int>(event->modifiers())).x());
  } else if(event->key() == Qt::Key_Up) {
    set_current_y(static_cast<double>(m_current_y->get()) +
      get_increment(static_cast<int>(event->modifiers())).y());
  } else if(event->key() == Qt::Key_Left) {
    set_current_x(static_cast<double>(m_current_x->get()) -
      get_increment(static_cast<int>(event->modifiers())).x());
  } else if(event->key() == Qt::Key_Down) {
    set_current_y(static_cast<double>(m_current_y->get()) -
      get_increment(static_cast<int>(event->modifiers())).y());
  } else if(event->key() == Qt::Key_Home) {
    set_current_x(*m_current_x->get_minimum());
    set_current_y(*m_current_y->get_minimum());
  } else if(event->key() == Qt::Key_End) {
    set_current_x(*m_current_x->get_maximum());
    set_current_y(*m_current_y->get_maximum());
  } else if(event->key() == Qt::Key_Escape) {
    set_current_x(m_submission.x());
    set_current_y(m_submission.y());
    m_is_modified = false;
  }
  QWidget::keyPressEvent(event);
}

void Slider2D::mouseMoveEvent(QMouseEvent* event) {
  if(m_is_mouse_down) {
    auto pos = m_track->get_body()->mapFromGlobal(event->globalPos());
    auto current_x = to_value_x(pos.x());
    auto current_y = to_value_y(pos.y());
    if(m_step_size != QPoint(0, 0)) {
      set_current_x(std::round(current_x / m_step_size.x()) * m_step_size.x());
      set_current_y(std::round(current_y / m_step_size.y()) * m_step_size.y());
    } else {
      set_current_x(current_x);
      set_current_y(current_y);
    }
  }
  QWidget::mouseMoveEvent(event);
}

void Slider2D::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_is_mouse_down = true;
    auto pos = m_track->get_body()->mapFromGlobal(event->globalPos());
    set_current_x(to_value_x(pos.x()));
    set_current_y(to_value_y(pos.y()));
  }
  QWidget::mousePressEvent(event);
}

void Slider2D::mouseReleaseEvent(QMouseEvent* event) {
  m_is_mouse_down = false;
  QWidget::mouseReleaseEvent(event);
}

void Slider2D::resizeEvent(QResizeEvent* event) {
  on_current_x(m_current_x->get());
  on_current_y(m_current_y->get());
  QWidget::resizeEvent(event);
}

void Slider2D::showEvent(QShowEvent* event) {
  on_current_x(m_current_x->get());
  on_current_y(m_current_y->get());
  QWidget::showEvent(event);
}

void Slider2D::wheelEvent(QWheelEvent* event) {
  auto increment = get_increment(Qt::NoModifier);
  set_current_x(
    m_current_x->get() + increment.x() * event->angleDelta().x() / 120.0);
  set_current_y(
    m_current_y->get() + increment.y() * event->angleDelta().y() / 120.0);
}

QPoint Slider2D::get_increment(int modifier_flag) const {
  if(auto increment =
      m_modifiers.find(static_cast<Qt::KeyboardModifier>(modifier_flag));
      increment != m_modifiers.end()) {
    return increment.value();
  }
  return m_modifiers[Qt::NoModifier];
}

double Slider2D::to_value_x(int x) const {
  return *m_current_x->get_minimum() + x * get_range(*m_current_x) /
    m_track->get_body()->width();
}

double Slider2D::to_value_y(int y) const {
  return *m_current_y->get_maximum() - y * get_range(*m_current_y) /
    m_track->get_body()->height();
}

int Slider2D::to_position_x(double x) const {
  auto track_body_pos = m_track->get_body()->mapTo(m_track, QPoint(0, 0));
  return static_cast<int>((x - *m_current_x->get_minimum()) /
    get_range(*m_current_x) * m_track->get_body()->width() -
      m_thumb->width() / 2.0) + track_body_pos.x();
}

int Slider2D::to_position_y(double y) const {
  auto track_body_pos = m_track->get_body()->mapTo(m_track, QPoint(0, 0));
  return static_cast<int>((*m_current_y->get_maximum() - y) /
    get_range(*m_current_y) * m_track->get_body()->height() -
      m_thumb->height() / 2.0) + track_body_pos.y();
}

void Slider2D::set_current_x(double x) {
  auto current_x = get_value(x, *m_current_x);
  if(current_x != m_current_x->get()) {
    m_current_x->set(current_x);
  }
}

void Slider2D::set_current_y(double y) {
  auto current_y = get_value(y, *m_current_y);
  if(current_y != m_current_y->get()) {
    m_current_y->set(current_y);
  }
}

void Slider2D::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    if(m_is_modified) {
      m_is_modified = false;
      m_submission = {m_current_x->get(), m_current_y->get()};
      m_submit_signal(m_submission);
    }
  }
}

void Slider2D::on_current_x(int x) {
  if(x < *m_current_x->get_minimum() || x > *m_current_x->get_maximum()) {
    return;
  }
  m_is_modified = true;
  m_thumb->move(to_position_x(x), m_thumb->y());
}

void Slider2D::on_current_y(int y) {
  if(y < *m_current_y->get_minimum() || y > *m_current_y->get_maximum()) {
    return;
  }
  m_is_modified = true;
  m_thumb->move(m_thumb->x(), to_position_y(y));
}

void Slider2D::on_track_style() {
  auto& stylist = find_stylist(*m_track);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          if(m_track_image == image) {
            return;
          }
          *has_update = true;
          m_track_image = image;
        });
      });
  }
  if(*has_update) {
    if(m_track_image.isNull()) {
      unmatch(*m_track, ImageTrack());
    } else {
      match(*m_track, ImageTrack());
    }
    m_track_image_container->setPixmap(QPixmap::fromImage(m_track_image));
    m_track_image_container->update();
  }
}

void Slider2D::on_thumb_icon_style() {
  auto& stylist = find_stylist(*m_thumb->get_body());
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          if(m_thumb_image == image) {
            return;
          }
          *has_update = true;
          m_thumb_image = image;
        });
      });
  }
  if(*has_update) {
    if(m_thumb_image.isNull()) {
      unmatch(*m_thumb, ImageThumb());
      m_thumb->setFixedSize(DEFAULT_THUMB_SIZE());
    } else {
      match(*m_thumb, ImageThumb());
      m_thumb->setFixedSize(m_thumb_image.size());
    }
  }
}
