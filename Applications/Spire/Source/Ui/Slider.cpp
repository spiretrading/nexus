#include "Spire/Ui/Slider.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::multiprecision;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using ImageTrack = StateSelector<void, struct ImageTrackSelectorTag>;
  using ImageThumb = StateSelector<void, struct ImageThumbSelectorTag>;

  auto DEFUALT_TRACK_WIDTH() {
    static auto width = scale_width(4);
    return width;
  }

  auto DEFUALT_TRACK_HEIGHT() {
    static auto height = scale_height(4);
    return height;
  }

  auto DEFAULT_THUMB_SIZE() {
    static auto size = scale(10, 26);
    return size;
  }

  auto HORIZONTAL_PADDING_SIZE() {
    static auto size = scale_width(5);
    return size;
  }

  auto VERTICAL_PADDING_SIZE() {
    static auto size = scale_height(5);
    return size;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > TrackRail()).
      set(BackgroundColor(QColor(0xC8C8C8)));
    style.get(Any() > (Track() && Hover()) > TrackRail()).
      set(BackgroundColor(QColor(0xA0A0A0)));
    style.get(Disabled() > TrackRail()).
      set(BackgroundColor(QColor(0xEBEBEB)));
    style.get(Any() > TrackFill()).
      set(BackgroundColor(QColor(0x8D78EC)));
    style.get(Any() > (Track() && (Hover() || Press())) > TrackFill()).
      set(BackgroundColor(QColor(0x4B23A0)));
    style.get(Disabled() > TrackFill()).
      set(BackgroundColor(QColor(0xA0A0A0)));
    style.get(matches(EnumProperty<Qt::Orientation>(Qt::Vertical)) > Track()).
      set(horizontal_padding(0)).
      set(vertical_padding(VERTICAL_PADDING_SIZE()));
    style.get(matches(EnumProperty<Qt::Orientation>(Qt::Horizontal)) > Track()).
      set(horizontal_padding(HORIZONTAL_PADDING_SIZE())).
      set(vertical_padding(0));
    style.get(Any() > ImageTrack()).
      set(horizontal_padding(0)).
      set(vertical_padding(0));
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
    style.get(Press() > Thumb()).
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

  void clear_fixed_size(QWidget& widget) {
    widget.setMinimumSize(0, 0);
    widget.setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  }

  auto make_modifiers(const ScalarValueModel<Decimal>& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, Decimal>();
    modifiers[Qt::NoModifier] = current.get_increment().get_value_or(1);
    return modifiers;
  }

  auto get_layout_direction(Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Vertical) {
      return QBoxLayout::LeftToRight;
    }
    return QBoxLayout::TopToBottom;
  }

  Decimal get_distance(Qt::Orientation orientation, const Decimal& value,
      const Decimal& minimum, const Decimal& maximum) {
    if(orientation == Qt::Orientation::Vertical) {
      return maximum - value;
    }
    return value - minimum;
  }

  Decimal get_value(Qt::Orientation orientation, const Decimal& distance,
      const Decimal& minimum, const Decimal& maximum) {
    if(orientation == Qt::Orientation::Vertical) {
      return maximum - distance;
    }
    return minimum + distance;
  }

  auto get_position(Qt::Orientation orientation, const QPoint& point) {
    if(orientation == Qt::Orientation::Vertical) {
      return point.y();
    }
    return point.x();
  }

  auto get_size(Qt::Orientation orientation, const QSize& size) {
    if(orientation == Qt::Orientation::Vertical) {
      return size.height();
    }
    return size.width();
  }

  auto get_thumb_size(Qt::Orientation orientation) {
    if(orientation == Qt::Orientation::Vertical) {
      return DEFAULT_THUMB_SIZE().transposed();
    }
    return DEFAULT_THUMB_SIZE();
  }

  Decimal ceil_value(const Decimal& value, const Decimal& step_size) {
    if(step_size != 0) {
      auto step = abs(step_size);
      return ceil(value / step) * step;
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

struct Slider::SliderValueModel : ScalarValueModel<Decimal> {
  std::shared_ptr<ScalarValueModel<Decimal>> m_source;
  scoped_connection m_current_connection;

  SliderValueModel(std::shared_ptr<ScalarValueModel<Decimal>> source)
    : m_source(std::move(source)),
      m_current_connection(m_source->connect_update_signal(
        std::bind_front(&SliderValueModel::on_current, this))) {}

  optional<Decimal> get_minimum() const override {
    return m_source->get_minimum().get_value_or(0);
  }

  optional<Decimal> get_maximum() const override {
    return m_source->get_maximum().get_value_or(100);
  }

  optional<Decimal> get_increment() const override {
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

Slider::Slider(QWidget* parent)
  : Slider(std::make_shared<LocalScalarValueModel<Decimal>>(), parent) {}

Slider::Slider(QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
  : Slider(std::make_shared<LocalScalarValueModel<Decimal>>(),
      std::move(modifiers), parent) {}

Slider::Slider(std::shared_ptr<ScalarValueModel<Decimal>> current,
  QWidget* parent)
  : Slider(std::move(current), make_modifiers(*current), parent) {}

Slider::Slider(std::shared_ptr<ScalarValueModel<Decimal>> current,
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
    : QWidget(parent),
      m_current(std::make_shared<SliderValueModel>(std::move(current))),
      m_modifiers(std::move(modifiers)),
      m_orientation(Qt::Horizontal),
      m_submission(m_current->get()),
      m_step(0),
      m_focus_observer(*this),
      m_is_mouse_down(false),
      m_is_modified(false) {
  setFocusPolicy(Qt::StrongFocus);
  auto track_body = new LayeredWidget();
  track_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_track_image_container = new QLabel();
  m_track_image_container->setScaledContents(true);
  m_track_image_container->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto track_rail = new Box(m_track_image_container);
  track_rail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*track_rail, TrackRail());
  track_body->add(track_rail);
  m_track_fill = new Box();
  m_track_fill->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*m_track_fill, TrackFill());
  track_body->add(m_track_fill);
  m_track = new Box(track_body);
  m_track->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*m_track, Track());
  link(*m_track, *track_rail);
  link(*m_track, *m_track_fill);
  m_body = new QWidget();
  m_track_layout = new QBoxLayout(get_layout_direction(m_orientation), m_body);
  m_track_layout->setContentsMargins({});
  m_track_layout->addStretch(1);
  m_track_layout->addWidget(m_track);
  m_track_layout->addStretch(1);
  auto thumb_icon = new Icon(QImage());
  m_thumb = new Box(thumb_icon, m_body);
  match(*m_thumb, Thumb());
  auto box = new Box(m_body);
  enclose(*this, *box);
  link(*this, *track_rail);
  link(*this, *m_track_fill);
  link(*this, *m_track);
  link(*this, *m_thumb);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  update_track();
  update_thumb();
  m_focus_observer.connect_state_signal(
    std::bind_front(&Slider::on_focus, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&Slider::on_current, this));
  m_track_style_connection = connect_style_signal(
    *m_track, std::bind_front(&Slider::on_track_style, this));
  m_thumb_icon_style_connection = connect_style_signal(
    *thumb_icon, std::bind_front(&Slider::on_thumb_icon_style, this));
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&Slider::on_style, this));
}

const std::shared_ptr<ScalarValueModel<Decimal>>& Slider::get_current() const {
  return m_current->m_source;
}

Decimal Slider::get_step() const {
  return m_step;
}

void Slider::set_step(const Decimal& step) {
  m_step = step;
}

connection Slider::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void Slider::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Right) {
    set_current(m_current->get() + ceil_value(
      get_increment(static_cast<int>(event->modifiers())), m_step));
  } else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Left) {
    set_current(m_current->get() - ceil_value(
      get_increment(static_cast<int>(event->modifiers())), m_step));
  } else if(event->key() == Qt::Key_PageUp) {
    set_current(m_current->get() +
      ceil_value(get_increment(Qt::ShiftModifier), m_step));
  } else if(event->key() == Qt::Key_PageDown) {
    set_current(m_current->get() -
      ceil_value(get_increment(Qt::ShiftModifier), m_step));
  } else if(event->key() == Qt::Key_Home) {
    m_current->set(*m_current->get_minimum());
  } else if(event->key() == Qt::Key_End) {
    m_current->set(*m_current->get_maximum());
  } else if(event->key() == Qt::Key_Escape) {
    set_current(m_submission);
    m_is_modified = false;
  }
  QWidget::keyPressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent* event) {
  if(m_is_mouse_down) {
    set_current(round_value(to_value(::get_position(m_orientation,
      m_track->get_body()->mapFromGlobal(event->globalPos()))), m_step));
  }
  QWidget::mouseMoveEvent(event);
}

void Slider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_is_mouse_down = true;
    match(*this, Press());
    set_current(round_value(to_value(::get_position(m_orientation,
      m_track->get_body()->mapFromGlobal(event->globalPos()))), m_step));
  }
  QWidget::mousePressEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent* event) {
  m_is_mouse_down = false;
  unmatch(*this, Press());
  QWidget::mouseReleaseEvent(event);
}

void Slider::resizeEvent(QResizeEvent* event) {
  on_current(m_current->get());
  QWidget::resizeEvent(event);
}

void Slider::showEvent(QShowEvent* event) {
  on_current(m_current->get());
  QWidget::showEvent(event);
}

void Slider::wheelEvent(QWheelEvent* event) {
  auto direction = [&] {
    if(event->angleDelta().y() < 0) {
      return -1;
    }
    return 1;
  }();
  auto increment = get_increment(Qt::NoModifier);
  set_current(m_current->get() + direction *
    ceil_value(increment * direction * event->angleDelta().y() / 120.0,
      m_step));
}

Decimal Slider::get_increment(int modifier_flag) const {
  if(auto increment =
      m_modifiers.find(static_cast<Qt::KeyboardModifier>(modifier_flag));
      increment != m_modifiers.end()) {
    return increment.value();
  }
  return m_modifiers[Qt::NoModifier];
}

Decimal Slider::to_value(int position) const {
  auto distance = position *
    (*m_current->get_maximum() - *m_current->get_minimum()) /
      get_size(m_orientation, m_track->get_body()->size());
  return get_value(m_orientation, distance, *m_current->get_minimum(),
    *m_current->get_maximum());
}

int Slider::to_position(const Decimal& value) const {
  auto distance = get_distance(m_orientation, value, *m_current->get_minimum(),
    *m_current->get_maximum());
  auto track_body_pos = m_track->get_body()->mapTo(m_body, QPoint(0, 0));
  return static_cast<int>(distance /
    (*m_current->get_maximum() - *m_current->get_minimum()) *
      get_size(m_orientation, m_track->get_body()->size()) -
        get_size(m_orientation, m_thumb->size()) / 2.0 +
          get_position(m_orientation, track_body_pos));
}

void Slider::set_current(const Decimal& value) {
  auto current = fmin(fmax(value, *m_current->get_minimum()),
    *m_current->get_maximum());
  if(current != m_current->get()) {
    m_current->set(current);
  }
}

void Slider::update_track() {
  clear_fixed_size(*m_track->get_body());
  clear_fixed_size(*m_track_fill);
  if(m_track_image.isNull()) {
    m_track_fill->show();
    if(m_orientation == Qt::Orientation::Vertical) {
      m_track->get_body()->setFixedWidth(DEFUALT_TRACK_WIDTH());
    } else {
      m_track->get_body()->setFixedHeight(DEFUALT_TRACK_HEIGHT());
    }
  } else {
    m_track_fill->hide();
  }
}

void Slider::update_thumb() {
  if(m_thumb_image.isNull()) {
    m_thumb->setFixedSize(get_thumb_size(m_orientation));
  } else {
    m_thumb->setFixedSize(m_thumb_image.size());
  }
}

void Slider::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && m_is_modified) {
    m_is_modified = false;
    m_submission = m_current->get();
    m_submit_signal(m_submission);
  }
}

void Slider::on_current(const Decimal& current) {
  if(current < *m_current->get_minimum() ||
      current > *m_current->get_maximum()) {
    return;
  }
  m_is_modified = true;
  if(m_orientation == Qt::Vertical) {
    m_thumb->move((m_body->width() - m_thumb->width()) / 2, to_position(current));
    if(m_track_fill->isVisible()) {
      m_track_fill->move(0, m_thumb->y());
      m_track_fill->setFixedHeight(
        m_track->get_body()->height() - m_track_fill->y());
    }
  } else {
    m_thumb->move(to_position(current), (m_body->height() - m_thumb->height()) / 2);
    if(m_track_fill->isVisible()) {
      m_track_fill->move(0, 0);
      m_track_fill->setFixedWidth(m_thumb->x());
    }
  }
}

void Slider::on_track_style() {
  auto& stylist = find_stylist(*m_track);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          if(image == m_track_image) {
            return;
          }
          *has_update = true;
          m_track_image = std::move(image);
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
    update_track();
  }
}

void Slider::on_thumb_icon_style() {
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
          m_thumb_image = std::move(image);
        });
      });
  }
  if(*has_update) {
    if(m_thumb_image.isNull()) {
      unmatch(*m_thumb, ImageThumb());
    } else {
      match(*m_thumb, ImageThumb());
    }
    update_thumb();
  }
}

void Slider::on_style() {
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<Qt::Orientation> orientation) {
        stylist.evaluate(orientation, [=] (auto orientation) {
          if(m_orientation == orientation) {
            return;
          }
          m_orientation = orientation;
          m_track_image_container->setSizePolicy(
            m_track_image_container->sizePolicy().transposed());
          m_track_layout->setDirection(get_layout_direction(m_orientation));
          m_thumb->setFixedSize(m_thumb->size().transposed());
          update_track();
        });
      });
  }
}
