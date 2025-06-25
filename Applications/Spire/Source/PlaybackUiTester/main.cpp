#include <QApplication>
#include <QFormLayout>
#include <QTextEdit>
#include <QTimeEdit>
#include <QVBoxLayout>
#include "Spire/Playback/PlaybackControlPanel.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

namespace {
  using PlayStateModel = ValueModel<PlaybackControlPanel::State>;
  using LocalPlayStateModel = LocalValueModel<PlaybackControlPanel::State>;

  auto to_qt_time(const time_duration& duration) {
    return QTime::fromMSecsSinceStartOfDay(duration.total_milliseconds());
  }

  auto to_text(PlaybackControlPanel::State state) {
    if(state == PlaybackControlPanel::State::REPLAYING) {
      static const auto value = QObject::tr("Replaying");
      return value;
    } else if(state == PlaybackControlPanel::State::PAUSED) {
      static const auto value = QObject::tr("Paused");
      return value;
    } else {
      static const auto value = QObject::tr("Real Time");
      return value;
    }
  }

  class UtcTimeClient {
    public:
      UtcTimeClient() = default;

      ptime GetTime() {
        return microsec_clock::universal_time();
      }

      void Close() {}
  };
}

struct DemoPlaybackController : private QObject {
  PlaybackControlPanel m_control_panel;
  std::shared_ptr<BooleanModel> m_timer_enabled;
  std::shared_ptr<ValueModel<PlaybackControlPanel::State>> m_play_state;
  QTimer m_timer;

  DemoPlaybackController(std::shared_ptr<TimelineModel> timeline,
      TimeClientBox time_client, std::shared_ptr<DurationModel> playhead,
      std::shared_ptr<PlaybackSpeedModel> speed)
      : m_control_panel(PlaybackControlPanel(std::move(timeline),
          std::move(time_client), std::move(playhead), std::move(speed),
          microsec_clock::universal_time().date() - months(6))),
        m_play_state(std::make_shared<LocalPlayStateModel>(
          PlaybackControlPanel::State::PAUSED)),
        m_timer_enabled(std::make_shared<LocalBooleanModel>(false)),
BEAM_SUPPRESS_THIS_INITIALIZER()
        m_timer(this) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_play_state->connect_update_signal(
      std::bind_front(&DemoPlaybackController::on_state_update, this));
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout,
      std::bind_front(&DemoPlaybackController::on_timeout, this));
  }

  void on_state_update(PlaybackControlPanel::State state) {
    if(state == PlaybackControlPanel::State::PAUSED) {
      m_timer.stop();
    } else if(m_timer_enabled->get()) {
      m_timer.start();
    }
  }

  void on_timeout() {
    auto& timeline = m_control_panel.get_timeline()->get();
    if(m_play_state->get() == PlaybackControlPanel::State::REAL_TIME) {
      auto now = m_control_panel.get_time_client().GetTime();
      m_control_panel.get_playhead()->set(now - timeline.m_start);
      auto next_second = 1000 - (now.time_of_day().total_milliseconds() % 1000);
      m_timer.start(next_second);
    } else {
      auto duration = std::min(timeline.m_duration,
        m_control_panel.get_time_client().GetTime() - timeline.m_start);
      m_control_panel.get_playhead()->set(
        std::min(m_control_panel.get_playhead()->get() +
          milliseconds(static_cast<long long>(m_control_panel.get_speed()->get()
            * 1000)), duration));
    }
  }
};

struct PlaybackControlPanelTester : QWidget {
  DemoPlaybackController* m_controller;
  QDateTimeEdit* m_start_time;
  QTimeEdit* m_playhead;
  QTextEdit* m_event_log;
  scoped_connection m_timeline_connection;
  scoped_connection m_playhead_connection;
  scoped_connection m_state_connection;

  PlaybackControlPanelTester(DemoPlaybackController& controller)
      : m_controller(&controller) {
    setAttribute(Qt::WA_ShowWithoutActivating);
    auto layout = new QVBoxLayout(this);
    auto form_layout = new QFormLayout();
    m_start_time = new QDateTimeEdit(
      QDateTime(QDateTime::currentDateTimeUtc().date(), QTime(0, 0, 0)));
    m_start_time->setDisplayFormat("yyyy/MM/dd hh:mm:ss.zzz");
    m_start_time->setMaximumDateTime(QDateTime::currentDateTimeUtc());
    m_start_time->setTimeSpec(Qt::UTC);
    m_start_time->setCalendarPopup(true);
    form_layout->addRow("Start Time(UTC):", m_start_time);
    connect(m_start_time, &QDateTimeEdit::dateTimeChanged,
      std::bind_front(&PlaybackControlPanelTester::on_date_time_changed, this));
    auto duration = new QTimeEdit(QTime(8, 0, 0));
    duration->setDisplayFormat("hh:mm:ss:zzz");
    connect(duration, &QTimeEdit::timeChanged,
      std::bind_front(&PlaybackControlPanelTester::on_duration_changed, this));
    form_layout->addRow("Duration:", duration);
    m_playhead = new QTimeEdit(QTime(0, 0, 0));
    m_playhead->setDisplayFormat("hh:mm:ss:zzz");
    connect(m_playhead, &QTimeEdit::timeChanged,
      std::bind_front(&PlaybackControlPanelTester::on_playhead_changed, this));
    form_layout->addRow("Current:", m_playhead);
    auto timer_check_box = new CheckBox(m_controller->m_timer_enabled);
    form_layout->addRow("Enable Timer:", timer_check_box);
    layout->addLayout(form_layout);
    m_event_log = new QTextEdit();
    m_event_log->setReadOnly(true);
    layout->addWidget(m_event_log);
    m_timeline_connection =
      m_controller->m_control_panel.get_timeline()->connect_update_signal(
        std::bind_front(&PlaybackControlPanelTester::on_timeline_update, this));
    m_playhead_connection =
      m_controller->m_control_panel.get_playhead()->connect_update_signal(
        std::bind_front(&PlaybackControlPanelTester::on_playhead_update, this));
    m_state_connection = m_controller->m_control_panel.connect_state_signal(
      std::bind_front(&PlaybackControlPanelTester::on_state_update, this));
    m_controller->m_control_panel.installEventFilter(this);
  }

  const std::shared_ptr<TimelineModel>& get_timeline() const {
    return m_controller->m_control_panel.get_timeline();
  }

  const std::shared_ptr<DurationModel>& get_playhead() const {
    return m_controller->m_control_panel.get_playhead();
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  void showEvent(QShowEvent* event) override {
    m_controller->m_control_panel.show();
    m_controller->m_control_panel.adjustSize();
    move(m_controller->m_control_panel.pos() + QPoint(0, 200));
    resize(500, 600);
  }

  void on_date_time_changed(const QDateTime& date_time) {
    if(to_ptime(date_time) != get_timeline()->get().m_start) {
      auto blocker = shared_connection_block(m_timeline_connection);
      get_timeline()->set(Timeline{to_ptime(date_time),
        get_timeline()->get().m_duration});
    }
    m_event_log->append(QString("Start Time(UTC): %1").
      arg(QString::fromStdString(to_simple_string(
        get_timeline()->get().m_start))));
  }

  void on_duration_changed(const QTime& time) {
    m_playhead->blockSignals(true);
    m_playhead->setMaximumTime(time);
    m_playhead->blockSignals(false);
    get_timeline()->set(
      Timeline{get_timeline()->get().m_start,
        to_time_duration(time)});
  }

  void on_playhead_changed(const QTime& time) {
    auto current = to_time_duration(time);
    if(current != get_playhead()->get()) {
      get_playhead()->set(current);
    }
  }

  void on_timeline_update(const Timeline& timeline) {
    m_start_time->setDateTime(to_qdate_time(timeline.m_start));
  }

  void on_playhead_update(const time_duration& playhead) {
    m_playhead->blockSignals(true);
    m_playhead->setTime(to_qt_time(playhead));
    m_playhead->blockSignals(false);
    m_event_log->append(QString("Current Time(UTC): %1").
      arg(QString::fromStdString(to_simple_string(
        get_timeline()->get().m_start + playhead))));
  }

  void on_state_update(PlaybackControlPanel::State state) {
    m_controller->m_play_state->set(state);
    m_event_log->append(QString("State: %1").arg(to_text(state)));
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Playback Ui Tester"));
  initialize_resources();
  auto controller = DemoPlaybackController(
    std::make_shared<LocalTimelineModel>(Timeline{
      ptime(microsec_clock::universal_time().date(), time_duration(4, 0, 0)),
        time_duration(8, 0, 0)}), TimeClientBox(UtcTimeClient()),
    std::make_shared<LocalDurationModel>(time_duration(0, 0, 0)),
    std::make_shared<LocalPlaybackSpeedModel>(1));
  auto tester = PlaybackControlPanelTester(controller);
  tester.show();
  application.exec();
}
