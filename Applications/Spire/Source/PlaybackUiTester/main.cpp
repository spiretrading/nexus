#include <QApplication>
#include <QFormLayout>
#include <QTextEdit>
#include <QTimeEdit>
#include <QVBoxLayout>
#include "Spire/Playback/SeekBar.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Slider.hpp"

using namespace Spire;
using namespace boost::posix_time;
using namespace boost::signals2;

namespace {
  auto to_time_duration(const QTime& time) {
    return milliseconds(time.msecsSinceStartOfDay());
  }

  auto to_qt_time(const time_duration& duration) {
    return QTime::fromMSecsSinceStartOfDay(duration.total_milliseconds());
  }

  auto to_ptime(const QDateTime& date_time) {
    QDate date = date_time.date();
    QTime time = date_time.time();
    return ptime(boost::gregorian::date(date.year(), date.month(), date.day()),
      to_time_duration(time));
  }
}

struct SeekBarTester : QWidget {
  SeekBar* m_seek_bar;
  QTimeEdit* m_current_editor;
  QTextEdit* m_event_log;
  scoped_connection m_current_connection;

  SeekBarTester() {
    auto layout = new QVBoxLayout(this);
    m_seek_bar = new SeekBar(std::make_shared<LocalTimelineModel>());
    layout->addWidget(m_seek_bar);
    auto form_layout = new QFormLayout();
    auto start_time_editor = new QDateTimeEdit(
      QDateTime(QDateTime::currentDateTimeUtc().date(), QTime(0, 0, 0)));
    start_time_editor->setDisplayFormat("yyyy/MM/dd hh:mm:ss.zzz");
    start_time_editor->setTimeSpec(Qt::UTC);
    start_time_editor->setCalendarPopup(true);
    form_layout->addRow("Start Time(UTC):", start_time_editor);
    QObject::connect(start_time_editor, &QDateTimeEdit::dateTimeChanged,
      std::bind_front(&SeekBarTester::on_date_time_changed, this));
    auto duration_editor = new QTimeEdit(QTime(8, 0, 0));
    duration_editor->setDisplayFormat("hh:mm:ss:zzz");
    QObject::connect(duration_editor, &QTimeEdit::timeChanged,
      std::bind_front(&SeekBarTester::on_duration_changed, this));
    form_layout->addRow("Duration:", duration_editor);
    m_current_editor = new QTimeEdit(QTime(0, 0, 0));
    m_current_editor->setDisplayFormat("hh:mm:ss:zzz");
    QObject::connect(m_current_editor, &QTimeEdit::timeChanged,
      std::bind_front(&SeekBarTester::on_current_editor_changed, this));
    form_layout->addRow("Current:", m_current_editor);
    layout->addLayout(form_layout);
    on_date_time_changed(start_time_editor->dateTime());
    on_duration_changed(duration_editor->time());
    on_current_editor_changed(m_current_editor->time());
    m_event_log = new QTextEdit();
    m_event_log->setReadOnly(true);
    m_current_connection = m_seek_bar->get_current()->connect_update_signal(
      std::bind_front(&SeekBarTester::on_current_update, this));
    layout->addWidget(m_event_log);
  }

  void on_date_time_changed(const QDateTime& date_time) {
    m_seek_bar->get_timeline()->set(Timeline{to_ptime(date_time),
      m_seek_bar->get_timeline()->get().m_duration});
  }

  void on_duration_changed(const QTime& time) {
    m_current_editor->blockSignals(true);
    m_current_editor->setMaximumTime(time);
    m_current_editor->blockSignals(false);
    m_seek_bar->get_timeline()->set(
      Timeline{m_seek_bar->get_timeline()->get().m_start,
        to_time_duration(time)});
  }

  void on_current_editor_changed(const QTime& time) {
    auto current = to_time_duration(time);
    if(current != m_seek_bar->get_current()->get()) {
      m_seek_bar->get_current()->set(current);
    }
  }

  void on_current_update(const time_duration& current) {
    m_current_editor->setTime(to_qt_time(current));
    m_event_log->append(QString("Current Time(UTC): %1").
      arg(QString::fromStdString(to_simple_string(
        m_seek_bar->get_timeline()->get().m_start + current))));
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Playback Ui Tester"));
  initialize_resources();
  auto tester = SeekBarTester();
  tester.show();
  tester.resize(600, 800);
  application.exec();
}
