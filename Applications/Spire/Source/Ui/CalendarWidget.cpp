#include "Spire/Ui/CalendarWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Scalar.hpp"

using namespace Spire;

CalendarWidget::CalendarWidget(Scalar date, QWidget* parent)
    : QWidget(parent),
      m_current_month(2),
      m_current_year(2020),
      m_calendar(QCalendar::System::Gregorian) {
  setFixedSize(scale(168, 178));
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(4), scale_height(7), scale_width(4),
    scale_height(4));
  auto month_widget = new MonthSpinBox(date, this);
  month_widget->setFixedHeight(scale_height(26));
  month_widget->setFixedSize(scale(158, 26));
  month_widget->connect_month_signal([=] (auto date) {
    on_month_changed(date);
  });
  layout->addWidget(month_widget);
  auto day_label_layout = new QHBoxLayout();
  day_label_layout->setSpacing(scale_width(3));
  day_label_layout->setContentsMargins({});
  add_day_label(day_label_layout, tr("S"));
  add_day_label(day_label_layout, tr("M"));
  add_day_label(day_label_layout, tr("T"));
  add_day_label(day_label_layout, tr("W"));
  add_day_label(day_label_layout, tr("T"));
  add_day_label(day_label_layout, tr("F"));
  add_day_label(day_label_layout, tr("S"));
  layout->addLayout(day_label_layout);
  layout->addSpacing(scale_height(4));
  m_calendar_layout = new QGridLayout();
  m_calendar_layout->setContentsMargins({});
  m_calendar_layout->setSpacing(0);
  m_calendar_layout->setHorizontalSpacing(scale_width(3));
  m_calendar_layout->setVerticalSpacing(scale_height(3));
  layout->addLayout(m_calendar_layout);
  set_date(date);
}

void CalendarWidget::set_date(Scalar date) {
  auto datetime = boost::posix_time::ptime(date);
  m_current_year = datetime.date().year();
  m_current_month = datetime.date().month();
  update_calendar();
}

void CalendarWidget::on_month_changed(Scalar date) {
  auto new_date = boost::posix_time::ptime(date).date();
  m_current_month = new_date.month();
  m_current_year = new_date.year();
  update_calendar();
}

void CalendarWidget::add_day_label(QLayout* layout, const QString& text) {
  auto label = new QLabel(text, this);
  label->setFixedSize(scale(20, 20));
  label->setAlignment(Qt::AlignCenter);
  label->setStyleSheet(QString(R"(
      background-color: #FFFFFF;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
    )").arg(scale_height(12)));
  layout->addWidget(label);
}

void CalendarWidget::update_calendar() {
  auto day_count = m_calendar.daysInMonth(m_current_month, m_current_year);
  auto first_day = QDate(m_current_year, m_current_month, 1);
  auto first_day_of_week = first_day.dayOfWeek(m_calendar);
  for(auto i = 0; i < 42; ++i) {
    if(i < first_day_of_week) {
      auto day = first_day.addDays(i - first_day_of_week);
      m_dates[i] = day;
    } else if(i > day_count + first_day_of_week - 1) {
      m_dates[i] = QDate(m_current_year, m_current_month,
        i - day_count - first_day_of_week + 1);
    } else {
      m_dates[i] = QDate(m_current_year, m_current_month,
        i + 1 - first_day_of_week);
    }
  }
  for(auto day = 0; day < 7; ++day) {
    for(auto week = 0; week < 6; ++week) {
      auto date = m_dates[day + week * 7];
      auto day_widget = new CalendarDayWidget(date, this);
      day_widget->setFixedSize(scale(20, 20));
      m_calendar_layout->addWidget(day_widget, week, day);
    }
  }
}
