#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QTextEdit>
#include <QTimeEdit>
#include <QVBoxLayout>
#include "Spire/Playback/ReplayAttachMenuButton.hpp"
#include "Spire/Playback/ReplayWindow.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  using PlayStateModel = ValueModel<ReplayWindow::State>;
  using LocalPlayStateModel = LocalValueModel<ReplayWindow::State>;

  auto to_qt_time(const time_duration& duration) {
    return QTime::fromMSecsSinceStartOfDay(duration.total_milliseconds());
  }

  auto to_text(ReplayWindow::State state) {
    if(state == ReplayWindow::State::REPLAYING) {
      static const auto value = QObject::tr("Replaying");
      return value;
    } else if(state == ReplayWindow::State::PAUSED) {
      static const auto value = QObject::tr("Paused");
      return value;
    } else {
      static const auto value = QObject::tr("Real Time");
      return value;
    }
  }

  auto to_text_with_identifier(const TargetMenuItem::Target& target) {
    return QString("%1 [ID:%2]").
      arg(to_text(target)).
      arg(QString::fromStdString(target.m_identifier));
  }

  auto populate_targets() {
    auto targets = std::vector<SelectableTarget>();
    auto identifier = 0;
    targets.push_back({{to_string(identifier++), "Book View", QColor(),
      Security(), 1}, false});
    targets.push_back({{to_string(identifier++), "", QColor(0xBF9541),
      Security(), 2}, false});
    targets.push_back({{to_string(identifier++), "", QColor(0x00BFA0),
      Security(), 2}, false});
    targets.push_back({{to_string(identifier++), "Book View", QColor(Qt::red),
      Security(), 1}, false});
    targets.push_back({{to_string(identifier++), "Time and Sales", QColor(),
      Security(), 1}, false});
    targets.push_back({{to_string(identifier++), "", QColor(0xFF7B00),
      ParseSecurity("ABX.TSX"), 3}, true});
    targets.push_back({{to_string(identifier++), "Book View", QColor(),
      ParseSecurity("ABX.TSX"), 1}, false});
    targets.push_back({{to_string(identifier++), "Time and Sales", QColor(),
      ParseSecurity("ABX.TSX"), 1}, false});
    targets.push_back({{to_string(identifier++), "", QColor(Qt::blue),
      ParseSecurity("ARE.TSX"), 2}, false});
    targets.push_back({{to_string(identifier++), "", QColor(0x993EF2),
      ParseSecurity("MFC.TSX"), 2}, false});
    targets.push_back({{to_string(identifier++), "Time and Sales",
      QColor(Qt::green), ParseSecurity("TD.TSX"), 2}, false});
    return targets;
  }

  auto make_check_box(std::shared_ptr<ListModel<SelectableTarget>> targets,
      int index) {
    auto check_box = new CheckBox(make_field_value_model(
      make_list_value_model(targets, index), &SelectableTarget::m_selected));
    check_box->set_label(to_text_with_identifier(targets->get(index).m_target));
    return check_box;
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
  ReplayWindow m_replay_window;
  std::shared_ptr<BooleanModel> m_timer_enabled;
  std::shared_ptr<ValueModel<ReplayWindow::State>> m_play_state;
  QTimer m_timer;

  DemoPlaybackController(std::shared_ptr<TimelineModel> timeline,
      TimeClientBox time_client, std::shared_ptr<DurationModel> playhead,
      std::shared_ptr<PlaybackSpeedModel> speed)
      : m_replay_window(ReplayWindow(std::move(timeline),
          std::move(time_client), std::move(playhead), std::move(speed),
          microsec_clock::universal_time().date() - months(6))),
        m_play_state(std::make_shared<LocalPlayStateModel>(
          ReplayWindow::State::PAUSED)),
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

  void on_state_update(ReplayWindow::State state) {
    if(state == ReplayWindow::State::PAUSED) {
      m_timer.stop();
    } else if(m_timer_enabled->get()) {
      m_timer.start();
    }
  }

  void on_timeout() {
    auto& timeline = m_replay_window.get_timeline()->get();
    if(m_play_state->get() == ReplayWindow::State::REAL_TIME) {
      auto now = m_replay_window.get_time_client().GetTime();
      m_replay_window.get_playhead()->set(now - timeline.m_start);
    } else {
      auto duration = std::min(timeline.m_duration,
        m_replay_window.get_time_client().GetTime() - timeline.m_start);
      m_replay_window.get_playhead()->set(
        std::min(m_replay_window.get_playhead()->get() +
          milliseconds(static_cast<long long>(m_replay_window.get_speed()->get()
            * 1000)), duration));
    }
  }
};

struct ReplayWindowTester : QWidget {
  DemoPlaybackController* m_controller;
  QDateTimeEdit* m_start_time;
  QTimeEdit* m_playhead;
  QTextEdit* m_event_log;
  scoped_connection m_timeline_connection;
  scoped_connection m_playhead_connection;
  scoped_connection m_state_connection;

  ReplayWindowTester(DemoPlaybackController& controller)
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
      std::bind_front(&ReplayWindowTester::on_date_time_changed, this));
    auto duration = new QTimeEdit(QTime(8, 0, 0));
    duration->setDisplayFormat("hh:mm:ss:zzz");
    connect(duration, &QTimeEdit::timeChanged,
      std::bind_front(&ReplayWindowTester::on_duration_changed, this));
    form_layout->addRow("Duration:", duration);
    m_playhead = new QTimeEdit(QTime(0, 0, 0));
    m_playhead->setDisplayFormat("hh:mm:ss:zzz");
    connect(m_playhead, &QTimeEdit::timeChanged,
      std::bind_front(&ReplayWindowTester::on_playhead_changed, this));
    form_layout->addRow("Current:", m_playhead);
    auto timer_check_box = new CheckBox(m_controller->m_timer_enabled);
    form_layout->addRow("Enable Timer:", timer_check_box);
    layout->addLayout(form_layout);
    m_event_log = new QTextEdit();
    m_event_log->setReadOnly(true);
    layout->addWidget(m_event_log);
    m_timeline_connection =
      m_controller->m_replay_window.get_timeline()->connect_update_signal(
        std::bind_front(&ReplayWindowTester::on_timeline_update, this));
    m_playhead_connection =
      m_controller->m_replay_window.get_playhead()->connect_update_signal(
        std::bind_front(&ReplayWindowTester::on_playhead_update, this));
    m_state_connection = m_controller->m_replay_window.connect_state_signal(
      std::bind_front(&ReplayWindowTester::on_state_update, this));
    m_controller->m_replay_window.installEventFilter(this);
  }

  const std::shared_ptr<TimelineModel>& get_timeline() const {
    return m_controller->m_replay_window.get_timeline();
  }

  const std::shared_ptr<DurationModel>& get_playhead() const {
    return m_controller->m_replay_window.get_playhead();
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }

  void showEvent(QShowEvent* event) override {
    m_controller->m_replay_window.show();
    m_controller->m_replay_window.adjustSize();
    move(m_controller->m_replay_window.pos() + QPoint(0, 200));
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
    m_start_time->blockSignals(true);
    m_start_time->setDateTime(to_qdate_time(timeline.m_start));
    m_start_time->blockSignals(false);
  }

  void on_playhead_update(const time_duration& playhead) {
    m_playhead->blockSignals(true);
    m_playhead->setTime(to_qt_time(playhead));
    m_playhead->blockSignals(false);
    m_event_log->append(QString("Current Time(UTC): %1").
      arg(QString::fromStdString(to_simple_string(
        get_timeline()->get().m_start + playhead))));
  }

  void on_state_update(ReplayWindow::State state) {
    m_controller->m_play_state->set(state);
    m_event_log->append(QString("State: %1").arg(to_text(state)));
  }
};

struct ReplayAttachMenuButtonTester : QWidget {
  std::shared_ptr<ListModel<SelectableTarget>> m_targets;
  std::vector<SelectableTarget> m_candidate_targets;
  QVBoxLayout* m_target_group_layout;

  ReplayAttachMenuButtonTester()
      : m_targets(std::make_shared<ArrayListModel<SelectableTarget>>()),
        m_candidate_targets(populate_targets()) {
    setAttribute(Qt::WA_ShowWithoutActivating);
    for(auto& target : m_candidate_targets) {
      m_targets->push(target);
    }
    auto layout = new QVBoxLayout(this);
    auto top_layout = new QHBoxLayout();
    layout->addLayout(top_layout);
    auto left_layout = new QVBoxLayout();
    auto candidates_group = new QGroupBox("Candidate Targets");
    auto candidates_group_layout = new QVBoxLayout(candidates_group);
    auto list_widget = new QListWidget();
    candidates_group_layout->addWidget(list_widget);
    left_layout->addWidget(candidates_group);
    top_layout->addLayout(left_layout, 1);
    auto right_layout = new QVBoxLayout();
    auto target_group = new QGroupBox("Targets");
    m_target_group_layout = new QVBoxLayout(target_group);
    right_layout->addWidget(target_group);
    top_layout->addLayout(right_layout, 1);
    for(auto i = 0; i < m_targets->get_size(); ++i) {
      list_widget->addItem(to_text_with_identifier(m_targets->get(i).m_target));
      auto list_item = list_widget->item(i);
      list_item->setFlags(list_item->flags() | Qt::ItemIsUserCheckable);
      list_item->setData(Qt::UserRole,
        QString::fromStdString(m_targets->get(i).m_target.m_identifier));
      list_item->setCheckState(Qt::Checked);
      m_target_group_layout->addWidget(make_check_box(m_targets, i));
    }
    m_target_group_layout->addStretch(1);
    auto menu_button = make_replay_attach_menu_button(m_targets);
    menu_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(menu_button, 0, Qt::AlignHCenter);
    connect(list_widget, &QListWidget::itemChanged,
      std::bind_front(&ReplayAttachMenuButtonTester::list_item_changed, this));
    m_targets->connect_operation_signal(
      std::bind_front(&ReplayAttachMenuButtonTester::on_operation, this));
  }

  void list_item_changed(QListWidgetItem* item) {
    if(item->checkState() == Qt::Checked) {
      auto index = item->listWidget()->row(item);
      if(index >= 0 && index < m_candidate_targets.size()) {
        m_targets->push(m_candidate_targets[index]);
      }
    } else {
      auto i = std::find_if(m_targets->begin(), m_targets->end(),
        [&] (const SelectableTarget& target) {
          return target.m_target.m_identifier ==
            item->data(Qt::UserRole).toString().toStdString();
        });
      if(i != m_targets->end()) {
        m_targets->remove(i);
      }
    }
  }

  void on_operation(const SelectableTargetListModel::Operation& operation) {
    visit(operation,
      [&] (const SelectableTargetListModel::AddOperation& operation) {
        m_target_group_layout->insertWidget(m_target_group_layout->count() - 1,
          make_check_box(m_targets, operation.m_index));
      },
      [&] (const SelectableTargetListModel::PreRemoveOperation& operation) {
        if(auto item = m_target_group_layout->takeAt(operation.m_index)) {
          delete item->widget();
          delete item;
        }
      });
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
  auto tester = ReplayWindowTester(controller);
  tester.show();
  auto menu_button_tester = ReplayAttachMenuButtonTester();
  menu_button_tester.show();
  menu_button_tester.resize(700, 500);
  menu_button_tester.move(
    tester.mapToGlobal(tester.rect().topRight()) + QPoint(10, 0));
  application.exec();
}
