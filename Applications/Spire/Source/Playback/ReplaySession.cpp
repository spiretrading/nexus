#include "Spire/Playback/ReplaySession.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/TransformValueModel.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto TICK_MS = 100;
  const auto PRESENT_TOLERANCE = milliseconds(200);

  time_duration get_present_playhead(const Timeline& timeline, ptime now) {
    return std::min(std::max(now - timeline.m_start, time_duration(0, 0, 0)),
      timeline.m_duration);
  }

  class PlayheadModel : public DurationModel {
    public:
      PlayheadModel(std::shared_ptr<TimelineModel> timeline,
        TimeClient time_client, time_duration playhead)
        : m_timeline(std::move(timeline)),
          m_time_client(std::move(time_client)),
          m_value(clamp(playhead)) {}

      boost::optional<time_duration> get_minimum() const override {
        return time_duration(0, 0, 0);
      }

      boost::optional<time_duration> get_maximum() const override {
        return get_present_playhead(
          m_timeline->get(), m_time_client.get_time());
      }

      const Type& get() const override {
        return m_value.get();
      }

      QValidator::State test(const Type& value) const override {
        return QValidator::State::Acceptable;
      }

      QValidator::State set(const Type& value) override {
        return m_value.set(clamp(value));
      }

      connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const override {
        return m_value.connect_update_signal(slot);
      }

    private:
      std::shared_ptr<TimelineModel> m_timeline;
      mutable TimeClient m_time_client;
      LocalValueModel<time_duration> m_value;

      time_duration clamp(time_duration playhead) const {
        return std::max(std::min(playhead, *get_maximum()), *get_minimum());
      }
  };

  class SpeedModel : public PlaybackSpeedModel {
    public:
      explicit SpeedModel(Decimal speed)
          : m_value(Decimal(1)) {
        set(speed);
      }

      const Type& get() const override {
        return m_value.get();
      }

      QValidator::State test(const Type& value) const override {
        if(value <= Decimal(0)) {
          return QValidator::State::Invalid;
        }
        return QValidator::State::Acceptable;
      }

      QValidator::State set(const Type& value) override {
        if(test(value) != QValidator::State::Acceptable) {
          return QValidator::State::Invalid;
        }
        return m_value.set(value);
      }

      connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const override {
        return m_value.connect_update_signal(slot);
      }

    private:
      LocalValueModel<Decimal> m_value;
  };
}

ReplaySession::ReplaySession(Timeline timeline, time_duration playhead,
    Decimal speed, TimeClient time_client)
    : m_time_client(std::move(time_client)),
      m_timeline(std::make_shared<LocalTimelineModel>(std::move(timeline))),
      m_playhead(
        std::make_shared<PlayheadModel>(m_timeline, m_time_client, playhead)),
      m_speed(std::make_shared<SpeedModel>(std::move(speed))),
      m_state(
        std::make_shared<LocalPlaybackStateModel>(PlaybackState::PAUSED)),
      m_state_view(make_transform_value_model(m_state, [] (auto state) {
        return state;
      })),
      m_position(std::make_shared<LocalValueModel<ptime>>(
        m_timeline->get().m_start + m_playhead->get())),
      m_position_view(
        make_transform_value_model(m_position, [] (const auto& position) {
          return position;
        })) {
  reset_anchor();
  QObject::connect(
    &m_timer, &QTimer::timeout, std::bind_front(&ReplaySession::on_tick, this));
  m_timeline_connection = m_timeline->connect_update_signal(
    std::bind_front(&ReplaySession::on_timeline, this));
  m_playhead_connection = m_playhead->connect_update_signal(
    std::bind_front(&ReplaySession::on_playhead, this));
  m_speed_connection = m_speed->connect_update_signal(
    std::bind_front(&ReplaySession::on_speed, this));
}

const std::shared_ptr<TimelineModel>& ReplaySession::get_timeline() const {
  return m_timeline;
}

const std::shared_ptr<DurationModel>& ReplaySession::get_playhead() const {
  return m_playhead;
}

const std::shared_ptr<ValueModel<ptime>>& ReplaySession::get_position() const {
  return m_position_view;
}

const std::shared_ptr<PlaybackSpeedModel>& ReplaySession::get_speed() const {
  return m_speed;
}

const std::shared_ptr<PlaybackStateModel>& ReplaySession::get_state() const {
  return m_state_view;
}

TimeClient ReplaySession::get_time_client() const {
  return m_time_client;
}

void ReplaySession::play() {
  if(m_state->get() != PlaybackState::PAUSED) {
    return;
  }
  if(auto next = get_playing_state(); next != PlaybackState::PAUSED) {
    set_state(next);
  }
}

void ReplaySession::pause() {
  if(m_state->get() != PlaybackState::PAUSED) {
    set_state(PlaybackState::PAUSED);
  }
}

bool ReplaySession::is_before_start() const {
  return m_time_client.get_time() < m_timeline->get().m_start;
}

bool ReplaySession::is_at_present() const {
  auto& timeline = m_timeline->get();
  auto now = m_time_client.get_time();
  if(now - timeline.m_start > timeline.m_duration) {
    return false;
  }
  return m_playhead->get() + PRESENT_TOLERANCE >=
    get_present_playhead(timeline, now);
}

bool ReplaySession::is_at_end() const {
  return m_playhead->get() >= m_timeline->get().m_duration;
}

PlaybackState ReplaySession::get_playing_state() const {
  if(is_before_start()) {
    return PlaybackState::PAUSED;
  } else if(is_at_present()) {
    return PlaybackState::REAL_TIME;
  } else if(is_at_end()) {
    return PlaybackState::PAUSED;
  }
  return PlaybackState::REPLAYING;
}

void ReplaySession::set_state(PlaybackState state) {
  if(state == m_state->get()) {
    return;
  }
  reset_anchor();
  if(state == PlaybackState::PAUSED) {
    m_timer.stop();
  } else if(!m_timer.isActive()) {
    m_timer.start(TICK_MS);
  }
  m_state->set(state);
}

void ReplaySession::reset_anchor() {
  m_anchor =
    Anchor(m_time_client.get_time(), m_playhead->get(), m_speed->get());
}

time_duration ReplaySession::get_playhead_at(ptime now) const {
  auto elapsed = milliseconds(((now - m_anchor.m_time).total_milliseconds() *
    m_anchor.m_speed).convert_to<std::int64_t>());
  return std::min(m_anchor.m_playhead + elapsed,
    get_present_playhead(m_timeline->get(), now));
}

void ReplaySession::update_position() {
  auto position = m_timeline->get().m_start + m_playhead->get();
  if(position != m_position->get()) {
    m_position->set(position);
  }
}

void ReplaySession::update_state() {
  if(m_state->get() == PlaybackState::PAUSED) {
    return;
  }
  if(auto next = get_playing_state(); next != m_state->get()) {
    set_state(next);
  }
}

void ReplaySession::on_tick() {
  auto now = m_time_client.get_time();
  auto playhead = [&] {
    if(m_state->get() == PlaybackState::REAL_TIME) {
      return get_present_playhead(m_timeline->get(), now);
    }
    return get_playhead_at(now);
  }();
  {
    auto blocker = shared_connection_block(m_playhead_connection);
    m_playhead->set(playhead);
  }
  if(m_playhead->get() != playhead) {
    reset_anchor();
  }
  update_position();
  update_state();
}

void ReplaySession::on_timeline(const Timeline& timeline) {
  if(auto present_playhead =
      get_present_playhead(timeline, m_time_client.get_time());
      m_playhead->get() > present_playhead) {
    auto blocker = shared_connection_block(m_playhead_connection);
    m_playhead->set(present_playhead);
  }
  reset_anchor();
  update_position();
  update_state();
}

void ReplaySession::on_playhead(const time_duration& playhead) {
  reset_anchor();
  update_position();
  update_state();
}

void ReplaySession::on_speed(const Decimal& speed) {
  if(m_state->get() == PlaybackState::REPLAYING) {
    auto now = m_time_client.get_time();
    m_anchor = Anchor(now, get_playhead_at(now), speed);
  }
}
