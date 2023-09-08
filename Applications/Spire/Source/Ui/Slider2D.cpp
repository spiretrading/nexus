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

  auto make_local_scalar_value_model() {
    auto model = std::make_shared<LocalScalarValueModel<Decimal>>();
    model->set_increment(std::numeric_limits<Decimal>::epsilon());
    return model;
  }

  auto make_modifiers(const ScalarValueModel<Decimal>& model) {
    auto modifiers = QHash<Qt::KeyboardModifier, Decimal>();
    modifiers[Qt::NoModifier] = 1;
    return modifiers;
  }

  auto get_increment(int modifier_flag,
      const QHash<Qt::KeyboardModifier, Decimal>& modifiers) {
    if(auto increment =
        modifiers.find(static_cast<Qt::KeyboardModifier>(modifier_flag));
        increment != modifiers.end()) {
      return increment.value();
    }
    return modifiers[Qt::NoModifier];
  }

  auto get_range(const ScalarValueModel<Decimal>& model) {
    return *model.get_maximum() - *model.get_minimum();
  }

  void set_value(ScalarValueModel<Decimal>& model, const Decimal& value) {
    auto new_value = fmin(fmax(value, *model.get_minimum()),
      *model.get_maximum());
    if(new_value != model.get()) {
      model.set(new_value);
    }
  }

  Decimal ceil_value(const Decimal& value, const Decimal& step_size) {
    if(step_size != 0) {
      auto step = abs(step_size);
      auto result = ceil(abs(value) / step) * step;
      if(value < 0) {
        return -result;
      }
      return result;
    }
    return value;
  }

  Decimal round_value(const Decimal& value, const Decimal& step_size) {
    if(step_size != 0) {
      auto step = abs(step_size);
      return round(value / step) * step;
    }
    return value;
  }
}

struct Slider2D::SliderValueModel : ScalarValueModel<Decimal> {
  std::shared_ptr<ScalarValueModel<Decimal>> m_source;
  scoped_connection m_current_connection;

  SliderValueModel(std::shared_ptr<ScalarValueModel<Decimal>> source)
    : m_source(std::move(source)),
      m_current_connection(m_source->connect_update_signal(
        std::bind_front(&SliderValueModel::on_current, this))) {}

  optional<Decimal> get_minimum() const override {
    if(!m_source->get_minimum()) {
      return Decimal(0);
    }
    return *m_source->get_minimum();
  }

  optional<Decimal> get_maximum() const override {
    if(!m_source->get_maximum()) {
      return Decimal(100);
    }
    return *m_source->get_maximum();
  }

  Decimal get_increment() const override {
    return m_source->get_increment();
  }

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const Decimal& get() const override {
    return m_source->get();
  }

  QValidator::State test(const Decimal& value) const override {
    return m_source->test(value);
  }

  QValidator::State set(const Decimal& value) override {
    return m_source->set(value);
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_source->connect_update_signal(slot);
  }

  void on_current(const Decimal& current) {
    if(current < *get_minimum()) {
      set(*get_minimum());
    }
    if(current > *get_maximum()) {
      set(*get_maximum());
    }
  }
};

Slider2D::Slider2D(QWidget* parent)
  : Slider2D(make_local_scalar_value_model(), make_local_scalar_value_model(),
    parent) {}

Slider2D::Slider2D(QHash<Qt::KeyboardModifier, Decimal> x_modifiers,
  QHash<Qt::KeyboardModifier, Decimal> y_modifiers, QWidget* parent)
  : Slider2D(make_local_scalar_value_model(), make_local_scalar_value_model(),
    std::move(x_modifiers), std::move(y_modifiers), parent) {}

Slider2D::Slider2D(std::shared_ptr<ScalarValueModel<Decimal>> x_current,
  std::shared_ptr<ScalarValueModel<Decimal>> y_current, QWidget* parent)
  : Slider2D(std::move(x_current), std::move(y_current),
      make_modifiers(*x_current), make_modifiers(*y_current), parent) {}

Slider2D::Slider2D(std::shared_ptr<ScalarValueModel<Decimal>> x_current,
    std::shared_ptr<ScalarValueModel<Decimal>> y_current,
    QHash<Qt::KeyboardModifier, Decimal> x_modifiers,
    QHash<Qt::KeyboardModifier, Decimal> y_modifiers, QWidget* parent)
    : QWidget(parent),
      m_x_current(std::make_shared<SliderValueModel>(std::move(x_current))),
      m_y_current(std::make_shared<SliderValueModel>(std::move(y_current))),
      m_x_modifiers(std::move(x_modifiers)),
      m_y_modifiers(std::move(y_modifiers)),
      m_x_submission(m_x_current->get()),
      m_y_submission(m_y_current->get()),
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
  m_x_current_connection = m_x_current->connect_update_signal(
    std::bind_front(&Slider2D::on_x_current, this));
  m_y_current_connection = m_y_current->connect_update_signal(
    std::bind_front(&Slider2D::on_y_current, this));
  m_track_style_connection = connect_style_signal(*m_track,
    std::bind_front(&Slider2D::on_track_style, this));
  m_thumb_icon_style_connection = connect_style_signal(*thumb_icon,
    std::bind_front(&Slider2D::on_thumb_icon_style, this));
}

const std::shared_ptr<ScalarValueModel<Decimal>>&
    Slider2D::get_x_current() const {
  return m_x_current->m_source;
}

const std::shared_ptr<ScalarValueModel<Decimal>>&
    Slider2D::get_y_current() const {
  return m_y_current->m_source;
}

const Decimal& Slider2D::get_x_step() const {
  return m_x_step;
}

void Slider2D::set_x_step(const Decimal& step) {
  m_x_step = step;
}

const Decimal& Slider2D::get_y_step() const {
  return m_y_step;
}

void Slider2D::set_y_step(const Decimal& step) {
  m_y_step = step;
}

connection Slider2D::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void Slider2D::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Right) {
    set_x_current(m_x_current->get() + ceil_value(get_increment(
      static_cast<int>(event->modifiers()), m_x_modifiers), m_x_step));
  } else if(event->key() == Qt::Key_Up) {
    set_y_current(m_y_current->get() + ceil_value(get_increment(
      static_cast<int>(event->modifiers()), m_y_modifiers), m_y_step));
  } else if(event->key() == Qt::Key_Left) {
    set_x_current(m_x_current->get() - ceil_value(get_increment(
      static_cast<int>(event->modifiers()), m_x_modifiers), m_x_step));
  } else if(event->key() == Qt::Key_Down) {
    set_y_current(m_y_current->get() - ceil_value(get_increment(
      static_cast<int>(event->modifiers()), m_y_modifiers), m_y_step));
  } else if(event->key() == Qt::Key_Home) {
    set_x_current(*m_x_current->get_minimum());
    set_y_current(*m_y_current->get_minimum());
  } else if(event->key() == Qt::Key_End) {
    set_x_current(*m_x_current->get_maximum());
    set_y_current(*m_y_current->get_maximum());
  } else if(event->key() == Qt::Key_Escape) {
    set_x_current(m_x_submission);
    set_y_current(m_y_submission);
    m_is_modified = false;
  }
  QWidget::keyPressEvent(event);
}

void Slider2D::mouseMoveEvent(QMouseEvent* event) {
  if(m_is_mouse_down) {
    auto pos = m_track->get_body()->mapFromGlobal(event->globalPos());
    set_x_current(round_value(to_x_value(pos.x()), m_x_step));
    set_y_current(round_value(to_y_value(pos.y()), m_y_step));
  }
  QWidget::mouseMoveEvent(event);
}

void Slider2D::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_is_mouse_down = true;
    auto pos = m_track->get_body()->mapFromGlobal(event->globalPos());
    set_x_current(round_value(to_x_value(pos.x()), m_x_step));
    set_y_current(round_value(to_y_value(pos.y()), m_y_step));
  }
  QWidget::mousePressEvent(event);
}

void Slider2D::mouseReleaseEvent(QMouseEvent* event) {
  m_is_mouse_down = false;
  QWidget::mouseReleaseEvent(event);
}

void Slider2D::resizeEvent(QResizeEvent* event) {
  on_x_current(m_x_current->get());
  on_y_current(m_y_current->get());
  QWidget::resizeEvent(event);
}

void Slider2D::showEvent(QShowEvent* event) {
  on_x_current(m_x_current->get());
  on_y_current(m_y_current->get());
  QWidget::showEvent(event);
}

void Slider2D::wheelEvent(QWheelEvent* event) {
  auto [x_change, y_change] = [&] {
    auto x_increment = get_increment(Qt::NoModifier, m_x_modifiers);
    auto y_increment = get_increment(Qt::NoModifier, m_y_modifiers);
    if(event->modifiers().testFlag(Qt::ShiftModifier)) {
      return std::tuple(Decimal(x_increment * event->angleDelta().y() / 120.0),
        Decimal(0));
    }
    return std::tuple(Decimal(x_increment * event->angleDelta().x() / 120.0),
      Decimal(y_increment * event->angleDelta().y() / 120.0));
  }();
  set_x_current(m_x_current->get() + ceil_value(x_change, m_x_step));
  set_y_current(m_y_current->get() + ceil_value(y_change, m_y_step));
}

Decimal Slider2D::to_x_value(int x) const {
  return *m_x_current->get_minimum() + x * get_range(*m_x_current) /
    m_track->get_body()->width();
}

Decimal Slider2D::to_y_value(int y) const {
  return *m_y_current->get_maximum() - y * get_range(*m_y_current) /
    m_track->get_body()->height();
}

int Slider2D::to_x_position(const Decimal& x) const {
  auto track_body_pos = m_track->get_body()->mapTo(m_track, QPoint(0, 0));
  return static_cast<int>((x - *m_x_current->get_minimum()) /
    get_range(*m_x_current) * m_track->get_body()->width() -
      m_thumb->width() / 2.0) + track_body_pos.x();
}

int Slider2D::to_y_position(const Decimal& y) const {
  auto track_body_pos = m_track->get_body()->mapTo(m_track, QPoint(0, 0));
  return static_cast<int>((*m_y_current->get_maximum() - y) /
    get_range(*m_y_current) * m_track->get_body()->height() -
      m_thumb->height() / 2.0) + track_body_pos.y();
}

void Slider2D::set_x_current(const Decimal& x) {
  set_value(*m_x_current, x);
}

void Slider2D::set_y_current(const Decimal& y) {
  set_value(*m_y_current, y);
}

void Slider2D::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    if(m_is_modified) {
      m_is_modified = false;
      m_x_submission = m_x_current->get();
      m_y_submission = m_y_current->get();
      m_submit_signal(m_x_submission, m_y_submission);
    }
  }
}

void Slider2D::on_x_current(const Decimal& x) {
  if(x < *m_x_current->get_minimum() || x > *m_x_current->get_maximum()) {
    return;
  }
  m_is_modified = true;
  m_thumb->move(to_x_position(x), m_thumb->y());
}

void Slider2D::on_y_current(const Decimal& y) {
  if(y < *m_x_current->get_minimum() || y > *m_y_current->get_maximum()) {
    return;
  }
  m_is_modified = true;
  m_thumb->move(m_thumb->x(), to_y_position(y));
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
