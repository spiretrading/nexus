#include "Spire/Ui/CalendarWidget.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::gregorian;
using namespace Spire;

CalendarWidget::CalendarWidget(const date& selected_date, QWidget* parent)
    : QWidget(parent),
      m_selected_date_widget(nullptr) {
  setCursor({QPixmap::fromImage(
    imageFromSvg(":/Icons/finger-cursor.svg", scale(18, 18))), 0, 0});
  setObjectName("calendar_widget");
  setStyleSheet(QString(R"(
    #calendar_widget {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    })").arg(scale_width(1)).arg(scale_height(1)));
  setFixedSize(scale(168, 201));
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(6), scale_height(7), scale_width(4),
    scale_height(4));
  m_month_spin_box = new MonthAndYearSpinBox(selected_date, this);
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
  if(date != m_selected_date) {
    m_calendar_model.set_month(date.month(), date.year());
    update_calendar(date);
  }
  m_selected_date = date;
  set_highlight();
}

void CalendarWidget::on_date_selected(const date& date) {
  if(date != m_selected_date) {
    if(date.month() != m_month_spin_box->get_date().month() &&
        date.year() != m_month_spin_box->get_date().month()) {
      update_calendar(date);
      m_month_spin_box->set_date(date);
    }
    m_date_signal(date);
  }
  m_selected_date = date;
  set_highlight();
}

void CalendarWidget::on_month_changed(const date& date) {
  update_calendar(date);
  if(std::tie(date.year(), date.month()) ==
      std::tie(m_selected_date.year(), m_selected_date.month())) {
    set_highlight();
  }
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

CalendarDayWidget* CalendarWidget::get_day_widget(
    const date& date) {
  auto [x, y] = m_calendar_model.get_pos(date);
  return static_cast<CalendarDayWidget*>(m_calendar_layout->itemAtPosition(
    y, x)->widget());
}

void CalendarWidget::set_highlight() {
  if(m_selected_date_widget) {
    m_selected_date_widget->remove_highlight();
  }
  m_selected_date_widget = get_day_widget(m_selected_date);
  m_selected_date_widget->set_highlight();
}

void CalendarWidget::update_calendar(const date& displayed_date) {
  m_calendar_model.set_month(displayed_date.month(), displayed_date.year());
  if(m_selected_date_widget) {
    m_selected_date_widget = nullptr;
  }
  while(auto item = m_calendar_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  for(auto day = 0; day < 7; ++day) {
    for(auto week = 0; week < 6; ++week) {
      auto date = m_calendar_model.get_date(week, day);
      auto day_widget = [&] () {
        if(date.month() != displayed_date.month()) {
            return new CalendarDayWidget(date, "#C8C8C8", this);
          }
          return new CalendarDayWidget(date, "#000000", this);
        }();
      day_widget->setFixedSize(scale(20, 20));
      m_calendar_layout->addWidget(day_widget, week, day);
      day_widget->connect_clicked_signal([=] (auto date) {
          on_date_selected(date);
        });
    }
  }
}
