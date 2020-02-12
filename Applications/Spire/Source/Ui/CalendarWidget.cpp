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
  m_month_spin_box = new MonthSpinBox(selected_date, this);
  m_month_spin_box->setFixedHeight(scale_height(26));
  m_month_spin_box->setFixedSize(scale(158, 26));
  m_month_spin_box->connect_month_signal([=] (auto date) {
    on_month_changed(date);
  });
  layout->addWidget(m_month_spin_box);
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
  update_calendar(m_selected_date);
}

void CalendarWidget::on_date_selected(CalendarDayWidget* selected_widget) {
  if(selected_widget->get_date() != m_selected_date) {
    if(m_selected_date_widget) {
      m_selected_date_widget->remove_highlight();
    }
    m_selected_date_widget = selected_widget;
    m_selected_date_widget->set_highlight();
    auto last_selected_date = m_selected_date;
    m_selected_date = selected_widget->get_date();
    if(selected_widget->get_date().month() != last_selected_date.month()) {
      update_calendar(selected_widget->get_date());
      m_month_spin_box->set_date(selected_widget->get_date());
    }
    m_date_signal(m_selected_date);
  }
}

void CalendarWidget::on_month_changed(const date& date) {
  if(m_selected_date_widget) {
    m_selected_date_widget->remove_highlight();
  }
  update_calendar(date);
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

void CalendarWidget::update_calendar(const date& displayed_date) {
  auto first_day_of_month = date(displayed_date.year(), displayed_date.month(),
    1);
  auto day_count = first_day_of_month.end_of_month().day();
  auto first_day_of_week = first_day_of_month.day_of_week().as_number();
  for(auto i = 0; i < 42; ++i) {
    if(i < first_day_of_week) {
      m_dates[i] = first_day_of_month + days(i - first_day_of_week);
    } else if(i > day_count + first_day_of_week - 1) {
      m_dates[i] = date(displayed_date.year(),
        max(1, (displayed_date.month() + 1) % 13),
        i - day_count - first_day_of_week + 1);
    } else {
      m_dates[i] = date(displayed_date.year(), displayed_date.month(),
        i + 1 - first_day_of_week);
    }
  }
  for(auto day = 0; day < 7; ++day) {
    for(auto week = 0; week < 6; ++week) {
      auto date = m_dates[day + week * 7];
      auto day_widget = [&] () {
        if(date.month() != displayed_date.month()) {
          return new CalendarDayWidget(date, "#C8C8C8", this);
        }
        return new CalendarDayWidget(date, "#000000", this);
      }();
      day_widget->setFixedSize(scale(20, 20));
      qDebug() << QString("%1/%2/%3").arg(m_selected_date.day())
        .arg(m_selected_date.month()).arg(m_selected_date.year());
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
