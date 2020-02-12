#include "Spire/Ui/CalendarWidget.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::gregorian;
using namespace Spire;

CalendarWidget::CalendarWidget(const date& selected_date, QWidget* parent)
    : QWidget(parent),
      m_calendar(QCalendar::System::Gregorian),
      m_selected_date_widget(nullptr) {
  setStyleSheet("background-color: #FFFFFF;");
  setFixedSize(scale(168, 201));
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(6), scale_height(7), scale_width(4),
    scale_height(4));
  auto month_widget = new MonthSpinBox(selected_date, this);
  month_widget->setFixedHeight(scale_height(26));
  month_widget->setFixedSize(scale(158, 26));
  month_widget->connect_month_signal([=] (auto date) {
    on_month_changed(date);
  });
  layout->addWidget(month_widget);
  layout->addSpacing(scale_height(4));
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
  layout->addSpacing(scale_height(3));
  m_calendar_layout = new QGridLayout();
  m_calendar_layout->setContentsMargins({});
  m_calendar_layout->setSpacing(0);
  m_calendar_layout->setHorizontalSpacing(scale_width(3));
  m_calendar_layout->setVerticalSpacing(scale_height(3));
  layout->addLayout(m_calendar_layout);
  set_date(selected_date);
}

void CalendarWidget::set_date(const date& date) {
  m_selected_date = date;
  if(m_selected_date_widget) {
    m_selected_date_widget->remove_highlight();
  }
  update_calendar(m_selected_date.year(), m_selected_date.month());
}

void CalendarWidget::on_date_selected(CalendarDayWidget* selected_widget) {
  if(selected_widget->get_date() != m_selected_date) {
    m_selected_date = selected_widget->get_date();
    if(m_selected_date_widget) {
      m_selected_date_widget->remove_highlight();
    }
    m_selected_date_widget = selected_widget;
    m_selected_date_widget->set_highlight();
    m_date_signal(m_selected_date);
  }
}

void CalendarWidget::on_month_changed(const date& date) {
  if(m_selected_date_widget) {
    m_selected_date_widget->remove_highlight();
  }
  update_calendar(date.year(), date.month());
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

void CalendarWidget::update_calendar(int year, int month) {
  auto first_day_of_month = date(year, month, 1);
  auto day_count = first_day_of_month.end_of_month().day();
  auto first_day_of_week = first_day_of_month.day_of_week().as_number();
  for(auto i = 0; i < 42; ++i) {
    if(i < first_day_of_week) {
      m_dates[i] = first_day_of_month + days(i - first_day_of_week);
    } else if(i > day_count + first_day_of_week - 1) {
      m_dates[i] = date(year, max(1, (month + 1) % 13),
        i - day_count - first_day_of_week + 1);
    } else {
      m_dates[i] = date(year, month, i + 1 - first_day_of_week);
    }
  }
  for(auto day = 0; day < 7; ++day) {
    for(auto week = 0; week < 6; ++week) {
      auto date = m_dates[day + week * 7];
      auto day_widget = [&] () {
        if(date.month() != month) {
          return new CalendarDayWidget(date, "#C8C8C8", this);
        }
        return new CalendarDayWidget(date, "#000000", this);
      }();
      day_widget->setFixedSize(scale(20, 20));
      if(date == m_selected_date) {
        m_selected_date_widget = day_widget;
        m_selected_date_widget->set_highlight();
      }
      m_calendar_layout->addWidget(day_widget, week, day);
      day_widget->connect_clicked_signal([=] (auto date) {
          on_date_selected(day_widget);
        });
    }
  }
}
