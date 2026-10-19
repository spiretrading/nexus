#include "Spire/Playback/ReplayWindow.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QIcon>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_playback_icon_button(const QString& icon_path,
      const QString& tooltip) {
    auto button = make_icon_button(
      image_from_svg(icon_path, scale(26, 26)), tooltip);
    button->setFixedSize(scale(26, 26));
    return button;
  }

  auto make_play_toggle_button() {
    auto play_toggle = make_icon_toggle_button(
      image_from_svg(":/Icons/play.svg", scale(26, 26)),
      image_from_svg(":/Icons/pause.svg", scale(26, 26)),
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
    std::shared_ptr<TimelineModel> m_timeline;
    std::shared_ptr<DurationModel> m_playhead;
    LocalValueModel<optional<time_duration>> m_value;
    QValidator::State m_state;
    scoped_connection m_connection;

    PlaybackTimeModel(std::shared_ptr<TimelineModel> timeline,
        std::shared_ptr<DurationModel> playhead)
        : m_timeline(std::move(timeline)),
          m_playhead(std::move(playhead)),
          m_value(get_start_time() + m_playhead->get()),
          m_state(test(m_value.get())) {
      m_connection = m_playhead->connect_update_signal(
        std::bind_front(&PlaybackTimeModel::on_update, this));
    }

    optional<time_duration> get_minimum() const override {
      return get_start_time() + *m_playhead->get_minimum();
    }

    optional<time_duration> get_maximum() const override {
      return get_start_time() + *m_playhead->get_maximum();
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
      if(m_state == QValidator::Acceptable) {
        if(auto playhead = *value - get_start_time();
            playhead != m_playhead->get()) {
          m_playhead->set(playhead);
        }
      }
      return m_state;
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_value.connect_update_signal(slot);
    }

    time_duration get_start_time() const {
      return to_local_time(m_timeline->get().m_start).time_of_day();
    }

    void on_update(const time_duration& time) {
      m_value.set(get_start_time() + time);
    }
  };
}

ReplayWindow::ReplayWindow(std::shared_ptr<ReplaySession> session,
    std::shared_ptr<ListModel<PropertyHubMember*>> roster,
    std::shared_ptr<PropertyHubListModel> attachments,
    optional<date> min_date, QWidget* parent)
    : Window(parent),
      m_session(std::move(session)),
      m_attachments(std::move(attachments)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setWindowTitle(tr("Replay"));
  set_svg_icon(":/Icons/replay.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/replay.png"));
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  m_seek_bar =
    new SeekBar(m_session->get_timeline(), m_session->get_playhead());
  layout->addWidget(m_seek_bar);
  layout->addSpacing(scale_height(10));
  auto bottom_layout = make_hbox_layout();
  auto date_box = new DateBox(std::make_shared<PlaybackDateModel>(
    make_field_value_model(m_session->get_timeline(), &Timeline::m_start),
    std::move(min_date)));
  date_box->setFixedWidth(scale_width(118));
  bottom_layout->addWidget(date_box);
  bottom_layout->addSpacing(scale_width(8));
  auto time_box = new DurationBox(std::make_shared<PlaybackTimeModel>(
    m_session->get_timeline(), m_session->get_playhead()));
  time_box->setFixedWidth(scale_width(80));
  update_style(*time_box, [] (auto& style) {
    style.get(Any()).set(DurationFormat("hh:mm:ss"));
  });
  bottom_layout->addWidget(time_box);
  bottom_layout->addSpacing(scale_width(8));
  m_start_button = make_playback_icon_button(
    ":/Icons/start.svg", tr("Jump to Start (Home)"));
  m_start_button->connect_click_signal(
    std::bind_front(&ReplayWindow::on_jump_to_start_click, this));
  bottom_layout->addWidget(m_start_button);
  bottom_layout->addSpacing(scale_width(2));
  m_play_toggle = make_play_toggle_button();
  m_play_toggle->get_current()->connect_update_signal(
    std::bind_front(&ReplayWindow::on_play_checked, this));
  bottom_layout->addWidget(m_play_toggle);
  bottom_layout->addSpacing(scale_width(2));
  m_end_button = make_playback_icon_button(
    ":/Icons/end.svg", tr("Jump to End (End)"));
  m_end_button->connect_click_signal(
    std::bind_front(&ReplayWindow::on_jump_to_end_click, this));
  bottom_layout->addWidget(m_end_button);
  bottom_layout->addStretch();
  auto attach_button =
    make_replay_attach_menu_button(std::move(roster), m_attachments);
  attach_button->setFixedWidth(scale_width(32));
  bottom_layout->addWidget(attach_button);
  bottom_layout->addSpacing(scale_width(8));
  m_speed_box = make_playback_speed_box(m_session->get_speed());
  m_speed_box->setFixedWidth(scale_width(60));
  bottom_layout->addWidget(m_speed_box);
  layout->addLayout(bottom_layout);
  auto content = new Box(body);
  content->setFixedWidth(scale_width(512));
  update_style(*content, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  set_body(content);
  on_playhead_update(m_session->get_playhead()->get());
  on_state_update(m_session->get_state()->get());
  m_date_connection = date_box->connect_submit_signal(
    std::bind_front(&ReplayWindow::on_date_submit, this));
  m_playhead_connection = m_session->get_playhead()->connect_update_signal(
    std::bind_front(&ReplayWindow::on_playhead_update, this));
  m_state_connection = m_session->get_state()->connect_update_signal(
    std::bind_front(&ReplayWindow::on_state_update, this));
  m_timeline_connection = m_session->get_timeline()->connect_update_signal(
    std::bind_front(&ReplayWindow::on_timeline_update, this));
}

const std::shared_ptr<ReplaySession>& ReplayWindow::get_session() const {
  return m_session;
}

void ReplayWindow::closeEvent(QCloseEvent* event) {
  m_session->pause();
  clear(*m_attachments);
  Window::closeEvent(event);
}

void ReplayWindow::keyPressEvent(QKeyEvent* event) {
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

bool ReplayWindow::is_playhead_at_end() const {
  return m_session->get_playhead()->get() >=
    m_session->get_timeline()->get().m_duration;
}

void ReplayWindow::on_play_checked(bool checked) {
  if(checked) {
    m_session->play();
  } else {
    m_session->pause();
  }
  on_state_update(m_session->get_state()->get());
}

void ReplayWindow::on_jump_to_start_click() {
  m_session->get_playhead()->set(time_duration(0, 0, 0));
}

void ReplayWindow::on_jump_to_end_click() {
  m_session->get_playhead()->set(m_session->get_timeline()->get().m_duration);
}

void ReplayWindow::on_date_submit(const optional<date>& date) {
  if(m_play_toggle->isEnabled()) {
    m_play_toggle->get_current()->set(true);
  }
}

void ReplayWindow::on_playhead_update(const time_duration& playhead) {
  m_start_button->setDisabled(playhead.is_zero());
  m_end_button->setDisabled(is_playhead_at_end() ||
    m_session->get_state()->get() == PlaybackState::REAL_TIME);
  m_play_toggle->setDisabled(is_playhead_at_end());
}

void ReplayWindow::on_state_update(PlaybackState state) {
  auto is_playing = state != PlaybackState::PAUSED;
  if(m_play_toggle->get_current()->get() != is_playing) {
    m_play_toggle->get_current()->set(is_playing);
  }
  auto& tooltip = *m_play_toggle->findChild<Tooltip*>("Tooltip");
  if(state == PlaybackState::PAUSED) {
    tooltip.set_label(tr("Play (Space)"));
  } else {
    tooltip.set_label(tr("Pause (Space)"));
  }
  auto is_real_time = state == PlaybackState::REAL_TIME;
  m_speed_box->setDisabled(is_real_time);
  m_end_button->setDisabled(is_real_time || is_playhead_at_end());
}

void ReplayWindow::on_timeline_update(const Timeline& timeline) {
  auto is_valid = !timeline.m_start.is_special() &&
    timeline.m_duration.is_positive();
  m_start_button->setEnabled(is_valid);
  m_end_button->setEnabled(is_valid && !is_playhead_at_end() &&
    m_session->get_state()->get() != PlaybackState::REAL_TIME);
  m_play_toggle->setEnabled(is_valid && !is_playhead_at_end());
}
