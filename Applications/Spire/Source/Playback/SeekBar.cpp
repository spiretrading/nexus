#include "Spire/Playback/SeekBar.hpp"
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Slider.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto FADE_IN_MS = 100;

  auto make_default_modifiers() {
    static auto modifiers = QHash<Qt::KeyboardModifier, time_duration>();
    modifiers[Qt::NoModifier] = time_duration(0, 0, 10);
    modifiers[Qt::ShiftModifier] = time_duration(0, 1, 0);
    return modifiers;
  }

  auto to_decimal(const time_duration& duration) {
    return Decimal(duration.total_milliseconds());
  }

  auto to_time_duration(const Decimal& value) {
    return milliseconds(static_cast<long long>(value));
  }

  auto to_decimal_modifiers(
      const QHash<Qt::KeyboardModifier, time_duration>& modifiers) {
    auto decimal_modifiers = QHash<Qt::KeyboardModifier, Decimal>();
    for(auto i = modifiers.begin(); i != modifiers.end(); ++i) {
      decimal_modifiers[i.key()] = to_decimal(i.value());
    }
    return decimal_modifiers;
  }

  auto to_local_time(const ptime& utc_time) {
    if(utc_time.is_not_a_date_time()) {
      return utc_time;
    }
    return boost::date_time::c_local_adjustor<ptime>::utc_to_local(utc_time);
  }

  auto format_time(const time_duration& time_duration) {
    auto oss = std::ostringstream();
    oss << std::setfill('0')
      << std::setw(2) << time_duration.hours() << ':'
      << std::setw(2) << time_duration.minutes() << ':'
      << std::setw(2) << time_duration.seconds() << '.'
      << std::setw(3) << time_duration.fractional_seconds() / 1000;
    return QString::fromStdString(oss.str());
  }

  auto format_date_time(const ptime& time) {
    auto date = time.date();
    auto today = second_clock::local_time().date();
    auto oss = std::ostringstream();
    oss << std::setfill('0');
    if(today != date) {
      if(today.year() != date.year()) {
        oss << std::setw(4) << date.year() << '/';
      }
      oss << std::setw(2) << date.month().as_number() << '/'
        << std::setw(2) << date.day();
    }
    auto date_text = QString::fromStdString(oss.str());
    auto time_text = format_time(time.time_of_day());
    if(date_text.isEmpty()) {
      return time_text;
    }
    return date_text + " " + time_text;
  }

  auto make_tip() {
    auto tip = make_label("");
    tip->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    update_style(*tip, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0x333333))).
        set(TextColor(QColor(0xFFFFFF))).
        set(horizontal_padding(scale_width(6))).
        set(vertical_padding(scale_height(5)));
    });
    return tip;
  }
}

struct SeekBar::SliderPositionModel : ScalarValueModel<Decimal> {
  std::shared_ptr<TimelineModel> m_timeline;
  std::shared_ptr<DurationModel> m_current;
  LocalValueModel<Decimal> m_value;
  QValidator::State m_state;
  scoped_connection m_current_connection;
  scoped_connection m_timeline_connection;

  SliderPositionModel(std::shared_ptr<TimelineModel> timeline,
      std::shared_ptr<DurationModel> current)
      : m_timeline(std::move(timeline)),
        m_current(std::move(current)),
        m_value(::to_decimal(m_current->get())),
        m_state(test(m_value.get())) {
    m_current_connection = m_current->connect_update_signal(
      std::bind_front(&SliderPositionModel::on_current_update, this));
    m_timeline_connection = m_timeline->connect_update_signal(
      std::bind_front(&SliderPositionModel::on_timeline_update, this));
  }

  optional<Decimal> get_minimum() const override {
    return Decimal(0);
  }

  optional<Decimal> get_maximum() const override {
    return ::to_decimal(m_timeline->get().m_duration);
  }

  QValidator::State get_state() const override {
    return m_state;
  }

  const Decimal& get() const override {
    return m_value.get();
  }

  QValidator::State test(const Decimal& value) const override {
    if(value < get_minimum() || value > get_maximum()) {
      return QValidator::Invalid;
    }
    return m_current->test(to_time_duration(value));
  }

  QValidator::State set(const Decimal& value) override {
    m_state = test(value);
    if(m_state == QValidator::Invalid) {
      return QValidator::Invalid;
    }
    m_value.set(value);
    if(auto current = to_time_duration(value); current != m_current->get()) {
      m_current->set(to_time_duration(value));
    }
    return m_state;
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_value.connect_update_signal(slot);
  }

  void on_current_update(const time_duration& time) {
    m_value.set(::to_decimal(time));
  }

  void on_timeline_update(const Timeline& timeline) {
    if(m_current->get() > m_timeline->get().m_duration) {
      set(::to_decimal(m_timeline->get().m_duration));
    }
  }
};

SeekBar::SeekBar(std::shared_ptr<TimelineModel> timeline, QWidget* parent)
  : SeekBar(std::move(timeline),
      std::make_shared<LocalDurationModel>(time_duration(0, 0, 0)), parent) {}

SeekBar::SeekBar(std::shared_ptr<TimelineModel> timeline,
  std::shared_ptr<DurationModel> current, QWidget* parent)
  : SeekBar(std::move(timeline), std::move(current), make_default_modifiers(),
      parent) {}

SeekBar::SeekBar(std::shared_ptr<TimelineModel> timeline,
    std::shared_ptr<DurationModel> current,
    QHash<Qt::KeyboardModifier, time_duration> modifiers, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<SliderPositionModel>(std::move(timeline),
        std::move(current))),
      m_tip(nullptr) {
  m_slider = new Slider(m_model, to_decimal_modifiers(modifiers));
  m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFocusProxy(m_slider);
  enclose(*this, *m_slider);
  proxy_style(*this, *m_slider);
  update_style(*this, [] (auto& style) {
    style.get(Active() > is_a<Slider>() > Thumb()).set(Visibility::VISIBLE);
    style.get(!Active() > is_a<Slider>() > Thumb()).set(Visibility::NONE);
    style.get(!Active() > is_a<Slider>() > Track()).set(horizontal_padding(0));
  });
  m_hover_observer.emplace(*m_slider);
  m_hover_observer->connect_state_signal(
    std::bind_front(&SeekBar::on_hover, this));
}

const std::shared_ptr<TimelineModel>& SeekBar::get_timeline() const {
  return m_model->m_timeline;
}

const std::shared_ptr<DurationModel>& SeekBar::get_current() const {
  return m_model->m_current;
}

void SeekBar::on_mouse_move(QWidget& target, QMouseEvent& event) {
  m_tip->get_current()->set(
    format_date_time(to_local_time(m_model->m_timeline->get().m_start) +
      ::to_time_duration(m_slider->to_value(
        m_slider->mapFromGlobal(event.globalPos())))));
  m_tip->adjustSize();
  m_tip->move(std::clamp(event.globalX() - m_tip->width() / 2, 0,
    screen()->availableGeometry().width()),
    mapToGlobal(m_slider->pos()).y() - scale_height(4) - m_tip->height());
}

void SeekBar::on_hover(HoverObserver::State state) {
  if(!isActiveWindow()) {
    return;
  }
  if(state == HoverObserver::State::MOUSE_OVER) {
    if(!m_tip) {
      m_tip = make_tip();
    }
    auto animation = new QPropertyAnimation(m_tip, "windowOpacity", m_tip);
    animation->setDuration(FADE_IN_MS);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    m_tip->setWindowOpacity(0.0);
    m_tip->setVisible(true);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    m_mouse_move_observer.emplace(*m_slider);
    m_mouse_move_observer->connect_move_signal(
      std::bind_front(&SeekBar::on_mouse_move, this));
  } else if(m_tip) {
    m_tip->setVisible(false);
    m_mouse_move_observer.reset();
  }
}

ptime Spire::get_end_time(const Timeline& timeline) {
  return timeline.m_start + timeline.m_duration;
}
