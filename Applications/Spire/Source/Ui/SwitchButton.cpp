#include "Spire/Ui/SwitchButton.hpp"
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/CubicBezierExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Switch = StateSelector<void, struct SwitchTag>;
  using Track = StateSelector<void, struct TrackTag>;

  const auto ANIMATION_DURATION = milliseconds(200);
  const auto UNCHECKED_TRACK_COLOR = QColor(0xA0A0A0);

  auto SWITCH_BUTTON_SIZE() {
    static auto size = scale(24, 12);
    return size;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > Switch()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0));
    style.get(Any() > Track()).
      set(BackgroundColor(UNCHECKED_TRACK_COLOR)).
      set(border_size(0)).
      set(border_radius(scale_width(6))).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(2)));
    style.get((Disabled() || (Any() > is_a<Button>() && Press())) > Switch()).
      set(BackgroundColor(QColor(0xEBEBEB)));
    style.get(Disabled() > Track()).
      set(BackgroundColor(QColor(0xC8C8C8)));
    style.get((Disabled() && Checked()) > Track()).
      set(BackgroundColor(QColor(0xBAB3D9)));
    return style;
  }

  auto get_track_color(bool checked, bool is_focus_visible) {
    if(checked && is_focus_visible) {
      return QColor(0x4B23A0);
    } else if(checked) {
      return QColor(0x684BC7);
    } else if(is_focus_visible) {
      return QColor(0x808080);
    }
    return UNCHECKED_TRACK_COLOR;
  }

  void set_track_color(SwitchButton& switch_button, const QColor& color) {
    update_style(switch_button, [&] (auto& style) {
      style.get(Any() > Track()).set(BackgroundColor(color));
    });
  }
}

SwitchButton::SwitchButton(QWidget* parent)
  : SwitchButton(std::make_shared<LocalBooleanModel>(false), parent) {}

SwitchButton::SwitchButton(
    std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_is_layout_done(false) {
  auto track_body = new QWidget();
  track_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_track = new Box(track_body);
  match(*m_track, Track());
  link(*this, *m_track);
  m_switch = new Box(nullptr, track_body);
  match(*m_switch, Switch());
  link(*this, *m_switch);
  auto button = new Button(m_track);
  enclose(*this, *button);
  proxy_style(*this, *button);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(button);
  m_switch->move(get_switch_position(m_current->get()), 0);
  m_switch->show();
  m_focus_observer.emplace(*button);
  m_focus_observer->connect_state_signal(
    std::bind_front(&SwitchButton::on_focus, this));
  button->connect_click_signal(
    std::bind_front(&SwitchButton::on_click, this));
  m_connection = m_current->connect_update_signal(
    std::bind_front(&SwitchButton::on_current, this));
  track_body->installEventFilter(this);
  on_current(m_current->get());
  setFixedSize(SWITCH_BUTTON_SIZE());
}

const std::shared_ptr<BooleanModel>& SwitchButton::get_current() const {
  return m_current;
}

connection SwitchButton::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void SwitchButton::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    m_track_color_evaluator = boost::none;
    if(isEnabled()) {
      set_track_color(*this,
        get_track_color(m_current->get(), is_focus_visible()));
    }
  }
  QWidget::changeEvent(event);
}

bool SwitchButton::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_track->get_body() && event->type() == QEvent::Resize) {
    if(!m_is_layout_done) {
      m_is_layout_done = true;
      auto height = m_track->get_body()->height();
      if(height != m_switch->height()) {
        m_switch->setFixedSize(height, height);
        update_style(*this, [&] (auto& style) {
          style.get(Any() > Switch()).set(border_radius(height / 2));
        });
        m_switch->move(get_switch_position(m_current->get()), 0);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool SwitchButton::is_focus_visible() const {
  return m_focus_observer->get_state() == FocusObserver::State::FOCUS_VISIBLE;
}

void SwitchButton::resizeEvent(QResizeEvent* event) {
  if(event->size() != SWITCH_BUTTON_SIZE()) {
    setFixedSize(SWITCH_BUTTON_SIZE());
  }
  QWidget::resizeEvent(event);
}

int SwitchButton::get_switch_position(bool checked) const {
  if(checked) {
    return m_track->get_body()->width() - m_switch->width();
  }
  return 0;
}

void SwitchButton::animate_switch_position(bool checked) {
  if(!m_is_layout_done) {
    return;
  }
  auto target = get_switch_position(checked);
  m_switch_pos_evaluator.emplace(
    make_evaluator(ease(m_switch->x(), target, ANIMATION_DURATION),
      find_stylist(*m_switch)));
  m_switch_pos_evaluator->connect_evaluated_signal(
    [=] (auto x) {
      m_switch->move(x, 0);
    });
}

void SwitchButton::animate_track_color(const QColor& target) {
  if(!isEnabled()) {
    m_track_color_evaluator = boost::none;
    return;
  }
  auto start = [&] {
    auto& block = get_evaluated_block(*m_track);
    for(auto& property : block) {
      if(property.get_property_type() == typeid(BackgroundColor)) {
        return property.as<QColor>();
      }
    }
    return UNCHECKED_TRACK_COLOR;
  }();
  if(start == target) {
    return;
  }
  m_track_color_evaluator.emplace(
    make_evaluator(ease(start, target, ANIMATION_DURATION),
      find_stylist(*this)));
  m_track_color_evaluator->connect_evaluated_signal(
    [=] (const auto& color) {
      set_track_color(*this, color);
    });
}

void SwitchButton::on_click() {
  m_current->set(!m_current->get());
  m_submit_signal(m_current->get());
}

void SwitchButton::on_current(bool current) {
  if(is_match(*this, Checked()) == current) {
    return;
  }
  if(current) {
    match(*this, Checked());
  } else {
    unmatch(*this, Checked());
  }
  animate_track_color(get_track_color(current, is_focus_visible()));
  animate_switch_position(current);
}

void SwitchButton::on_focus(FocusObserver::State state) {
  animate_track_color(get_track_color(m_current->get(), is_focus_visible()));
}
