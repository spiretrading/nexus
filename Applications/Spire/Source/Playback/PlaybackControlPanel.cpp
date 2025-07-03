#include "Spire/Playback/PlaybackControlPanel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TIME_TOLERANCE = milliseconds(200);

  auto make_playback_icon_button(const QString& icon_path,
      const QString& tooltip) {
    auto button = make_icon_button(
      imageFromSvg(icon_path, scale(26, 26)), tooltip);
    button->setFixedSize(scale(26, 26));
    return button;
  }

  auto make_play_toggle_button() {
    auto play_toggle = make_icon_toggle_button(
      imageFromSvg(":/Icons/play.svg", scale(26, 26)),
      imageFromSvg(":/Icons/pause.svg", scale(26, 26)),
      std::make_shared<LocalBooleanModel>(false));
    play_toggle->setFixedSize(scale(26, 26));
    add_tooltip(QObject::tr("Play (Space)"), *play_toggle);
    update_style(*play_toggle, [] (auto& style) {
      style.get(Checked() > is_a<Icon>()).set(Fill(QColor(0x535353)));
      style.get((Checked() && Disabled()) > is_a<Icon>()).
        set(Fill(QColor(0xC8C8C8)));
    });
    return play_toggle;
  }

  struct PlaybackDateModel : OptionalDateModel {
    std::shared_ptr<ValueModel<ptime>> m_source;
    optional<date> m_min_date;
    LocalValueModel<optional<date>> m_value;
    QValidator::State m_state;
    scoped_connection m_connection;

    PlaybackDateModel(std::shared_ptr<ValueModel<ptime>> source,
        optional<date> min_date)
        : m_source(std::move(source)),
          m_min_date(std::move(min_date)),
          m_value(m_source->get().date()),
          m_state(test(m_value.get())) {
      m_connection = m_source->connect_update_signal(
        std::bind_front(&PlaybackDateModel::on_update, this));
    }

    optional<date> get_minimum() const override {
      return m_min_date;
    }

    optional<date> get_maximum() const override {
      return day_clock::local_day();
    }

    QValidator::State get_state() const override {
      return m_state;
    }

    const optional<date>& get() const override {
      return m_value.get();
    }

    QValidator::State test(const optional<date>& value) const override {
      if(!value || value < get_minimum() || value > get_maximum()) {
        return QValidator::Intermediate;
      }
      return QValidator::Acceptable;
    }

    QValidator::State set(const optional<date>& value) override {
      m_state = test(value);
      m_value.set(value);
      if(m_state == QValidator::Acceptable &&
          *value != m_source->get().date()) {
        m_source->set(ptime(*value, m_source->get().time_of_day()));
      }
      return m_state;
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_value.connect_update_signal(slot);
    }

    void on_update(const ptime& time) {
      m_value.set(time.date());
    }
  };

  struct PlaybackTimeModel : OptionalDurationModel {
    using local_adjustor = boost::date_time::c_local_adjustor<ptime>;
    std::shared_ptr<TimelineModel> m_timeline;
    mutable TimeClientBox m_time_client;
    std::shared_ptr<DurationModel> m_playhead;
    LocalValueModel<optional<time_duration>> m_value;
    QValidator::State m_state;
    scoped_connection m_connection;

    PlaybackTimeModel(std::shared_ptr<TimelineModel> timeline,
        TimeClientBox time_client, std::shared_ptr<DurationModel> playhead)
        : m_timeline(std::move(timeline)),
          m_time_client(std::move(time_client)),
          m_playhead(std::move(playhead)),
          m_value(get_start_time() + m_playhead->get()),
          m_state(test(m_value.get())) {
      m_connection = m_playhead->connect_update_signal(
        std::bind_front(&PlaybackTimeModel::on_update, this));
    }

    optional<time_duration> get_minimum() const override {
      return get_start_time();
    }

    optional<time_duration> get_maximum() const override {
      auto start_date =
        local_adjustor::utc_to_local(m_timeline->get().m_start).date();
      auto end_time = get_start_time() + m_timeline->get().m_duration;
      auto now = local_adjustor::utc_to_local(m_time_client.GetTime());
      if(now.date() > start_date) {
        return end_time;
      } else if(now.date() < start_date) {
        return get_minimum();
      }
      return std::min(end_time, now.time_of_day());
    }

    QValidator::State get_state() const override {
      return m_state;
    }

    const optional<time_duration>& get() const override {
      return m_value.get();
    }

    QValidator::State test(
        const optional<time_duration>& value) const override {
      if(!value || value < get_minimum() || value > get_maximum()) {
        return QValidator::Intermediate;
      }
      return QValidator::Acceptable;
    }

    QValidator::State set(const optional<time_duration>& value) override {
      m_state = test(value);
      m_value.set(value);
      if(auto playhead = *value - get_start_time();
          playhead != m_playhead->get()) {
        m_playhead->set(playhead);
      }
      return m_state;
    }

    connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
      return m_value.connect_update_signal(slot);
    }

    time_duration get_start_time() const {
      return local_adjustor::utc_to_local(
        m_timeline->get().m_start).time_of_day();
    }

    void on_update(const time_duration& time) {
      m_value.set(get_start_time() + time);
    }
  };
}

struct PlaybackControlPanel::PlayheadModel : DurationModel {
  mutable UpdateSignal m_update_signal;
  std::shared_ptr<TimelineModel> m_timeline;
  TimeClientBox m_time_client;
  std::shared_ptr<DurationModel> m_source;
  scoped_connection m_connection;

  PlayheadModel(std::shared_ptr<TimelineModel> timeline,
      TimeClientBox time_client, std::shared_ptr<DurationModel> source)
      : m_timeline(std::move(timeline)),
        m_time_client(std::move(time_client)),
        m_source(std::move(source)) {
    m_connection = m_source->connect_update_signal(
      std::bind_front(&PlayheadModel::on_update, this));
  }

  optional<time_duration> get_minimum() const override {
    return m_source->get_minimum();
  }

  optional<time_duration> get_maximum() const override {
    return m_source->get_maximum();
  }

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const time_duration& get() const override {
    return m_source->get();
  }

  QValidator::State test(const time_duration& value) const override {
    return m_source->test(value);
  }

  QValidator::State set(const time_duration& value) override {
    return m_source->set(std::min(value,
      std::max(m_time_client.GetTime() - m_timeline->get().m_start,
        time_duration(0, 0, 0))));
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_update_signal.connect(slot);
  }

  void on_update(const time_duration& value) {
    m_update_signal(value);
  }
};

PlaybackControlPanel::PlaybackControlPanel(
    std::shared_ptr<TimelineModel> timeline, TimeClientBox time_client,
    std::shared_ptr<DurationModel> playhead,
    std::shared_ptr<PlaybackSpeedModel> speed, optional<date> min_date,
    QWidget* parent)
    : QWidget(parent),
      m_playhead(std::make_shared<PlayheadModel>(std::move(timeline),
        std::move(time_client), std::move(playhead))),
      m_state(State::PAUSED) {
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addStretch();
  m_seek_bar = new SeekBar(get_timeline(), m_playhead);
  layout->addWidget(m_seek_bar);
  layout->addSpacing(scale_height(10));
  auto controls_body = new QWidget();
  auto controls_body_layout = make_hbox_layout(controls_body);
  auto date_box = new DateBox(std::make_shared<PlaybackDateModel>(
    make_field_value_model(get_timeline(), &Timeline::m_start),
    std::move(min_date)));
  date_box->setFixedWidth(scale_width(100));
  controls_body_layout->addWidget(date_box);
  controls_body_layout->addSpacing(scale_width(8));
  auto time_box = new DurationBox(std::make_shared<PlaybackTimeModel>(
    get_timeline(), m_playhead->m_time_client, m_playhead));
  time_box->setFixedWidth(scale_width(70));
  controls_body_layout->addWidget(time_box);
  controls_body_layout->addSpacing(scale_width(21));
  controls_body_layout->addStretch();
  m_start_button = make_playback_icon_button(
    ":/Icons/start.svg", tr("Jump to Start (Home)"));
  m_start_button->connect_click_signal(
    std::bind_front(&PlaybackControlPanel::on_jump_to_start_click, this));
  controls_body_layout->addWidget(m_start_button);
  controls_body_layout->addSpacing(scale_width(2));
  m_play_toggle = make_play_toggle_button();
  m_play_toggle->get_current()->connect_update_signal(
    std::bind_front(&PlaybackControlPanel::on_play_checked, this));
  controls_body_layout->addWidget(m_play_toggle);
  controls_body_layout->addSpacing(scale_width(2));
  m_end_button = make_playback_icon_button(
    ":/Icons/end.svg", tr("Jump to End (End)"));
  m_end_button->connect_click_signal(
    std::bind_front(&PlaybackControlPanel::on_jump_to_end_click, this));
  controls_body_layout->addWidget(m_end_button);
  controls_body_layout->addStretch();
  controls_body_layout->addSpacing(scale_width(65));
  m_speed_box = make_playback_speed_box(std::move(speed));
  m_speed_box->setFixedWidth(scale_width(60));
  controls_body_layout->addWidget(m_speed_box);
  auto controls_box = new Box(controls_body);
  layout->addWidget(controls_box);
  layout->addStretch();
  auto box = new Box(body);
  proxy_style(*this, *box);
  enclose(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  on_playhead_update(get_playhead()->get());
  on_state_update(m_state.get());
  m_state.connect_update_signal(
    std::bind_front(&PlaybackControlPanel::on_state_update, this));
  m_date_connection = date_box->connect_submit_signal(
    std::bind_front(&PlaybackControlPanel::on_date_submit, this));
  m_playhead_connection = m_playhead->connect_update_signal(
    std::bind_front(&PlaybackControlPanel::on_playhead_update, this));
  m_timeline_connection = get_timeline()->connect_update_signal(
    std::bind_front(&PlaybackControlPanel::on_timeline_update, this));
}

const std::shared_ptr<TimelineModel>&
    PlaybackControlPanel::get_timeline() const {
  return m_playhead->m_timeline;
}

TimeClientBox PlaybackControlPanel::get_time_client() const {
  return m_playhead->m_time_client;
}

const std::shared_ptr<DurationModel>&
    PlaybackControlPanel::get_playhead() const {
  return m_playhead->m_source;
}

const std::shared_ptr<PlaybackSpeedModel>&
    PlaybackControlPanel::get_speed() const {
  return m_speed_box->get_current();
}

connection PlaybackControlPanel::connect_state_signal(
    const StateSignal::slot_type& slot) const {
  return m_state.connect_update_signal(slot);
}

void PlaybackControlPanel::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Space && m_seek_bar->hasFocus() &&
      m_play_toggle->isEnabled()) {
    m_play_toggle->get_current()->set(!m_play_toggle->get_current()->get());
    event->accept();
  } else if(event->key() == Qt::Key_Home && m_start_button->isEnabled()) {
    on_jump_to_start_click();
    event->accept();
  } else if(event->key() == Qt::Key_End && m_end_button->isEnabled()) {
    on_jump_to_end_click();
    event->accept();
  } else {
    QWidget::keyPressEvent(event);
  }
}

bool PlaybackControlPanel::is_playhead_at_end() const {
  return get_playhead()->get() >= get_timeline()->get().m_duration;
}

void PlaybackControlPanel::play() {
  if(get_timeline()->get().m_start + m_playhead->get() <
      m_playhead->m_time_client.GetTime() - TIME_TOLERANCE) {
    if(m_state.get() != State::REPLAYING) {
      m_state.set(State::REPLAYING);
    }
  } else if(m_state.get() != State::REAL_TIME) {
    m_state.set(State::REAL_TIME);
  }
}

void PlaybackControlPanel::pause() {
  if(m_state.get() != State::PAUSED) {
    m_state.set(State::PAUSED);
  }
}

void PlaybackControlPanel::on_play_checked(bool checked) {
  if(checked) {
    play();
  } else {
    pause();
  }
}

void PlaybackControlPanel::on_jump_to_start_click() {
  m_playhead->set(time_duration(0, 0, 0));
}

void PlaybackControlPanel::on_jump_to_end_click() {
  if(get_end_time(get_timeline()->get()) <
      m_playhead->m_time_client.GetTime()) {
    m_playhead->set(get_timeline()->get().m_duration);
  } else {
    m_playhead->set(std::max(m_playhead->m_time_client.GetTime() -
      m_playhead->m_timeline->get().m_start, time_duration(0, 0, 0)));
  }
}

void PlaybackControlPanel::on_date_submit(const optional<date>& date) {
  if(m_play_toggle->isEnabled()) {
    m_play_toggle->get_current()->set(true);
  }
}

void PlaybackControlPanel::on_playhead_update(const time_duration& playhead) {
  if(is_playhead_at_end()) {
    m_play_toggle->get_current()->set(false);
  } else if(m_state.get() != State::PAUSED) {
    play();
  }
  m_start_button->setDisabled(playhead.is_zero());
  m_end_button->setDisabled(is_playhead_at_end() ||
    m_state.get() == State::REAL_TIME);
  m_play_toggle->setDisabled(is_playhead_at_end());
}

void PlaybackControlPanel::on_state_update(const State& state) {
  auto& tooltip = *m_play_toggle->findChild<Tooltip*>("Tooltip");
  if(state == State::PAUSED) {
    tooltip.set_label(tr("Play (Space)"));
  } else {
    tooltip.set_label(tr("Pause (Space)"));
  }
  auto is_real_time = state == State::REAL_TIME;
  m_speed_box->setDisabled(is_real_time);
  m_end_button->setDisabled(is_real_time || is_playhead_at_end());
}

void PlaybackControlPanel::on_timeline_update(const Timeline& timeline) {
  auto is_valid = !timeline.m_start.is_special() &&
    timeline.m_duration.is_positive();
  m_start_button->setEnabled(is_valid);
  m_end_button->setEnabled(is_valid && !is_playhead_at_end() &&
    m_state.get() != State::REAL_TIME);
  m_play_toggle->setEnabled(is_valid && !is_playhead_at_end());
}
