#include "Spire/Ui/DateInputWidget.hpp"
#include <QDate>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace Spire;

DateInputWidget::DateInputWidget(ptime date, QWidget* parent)
    : QLabel(parent) {
  setFocusPolicy(Qt::StrongFocus);
  setObjectName("dateinputwidget");
  setAlignment(Qt::AlignCenter);
  set_default_style();
  m_calendar_widget = new CalendarWidget(date.date(), this);
  m_calendar_widget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  m_calendar_widget->connect_date_signal([=] (auto date) {
      m_calendar_widget->hide();
      update_label(date);
    });
  auto shadow = std::make_unique<DropShadow>(false, m_calendar_widget);
  m_calendar_widget->hide();
  window()->installEventFilter(this);
  update_label(date.date());
}

bool DateInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_calendar_widget->isVisible()) {
        move_calendar();
      }
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_calendar_widget->hide();
    }
  } else if(watched == m_calendar_widget) {
    if(event->type() == QEvent::FocusOut) {
      m_calendar_widget->hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DateInputWidget::focusInEvent(QFocusEvent* event) {
  set_focus_style();
}

void DateInputWidget::focusOutEvent(QFocusEvent* event) {
  if(!m_calendar_widget->hasFocus()) {
    set_default_style();
    m_calendar_widget->hide();
  }
}

void DateInputWidget::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_calendar_widget->isVisible()) {
      m_calendar_widget->hide();
    } else {
      move_calendar();
      m_calendar_widget->show();
      m_calendar_widget->setFocus();
    }
  }
}

void DateInputWidget::moveEvent(QMoveEvent* event) {
  move_calendar();
}

void DateInputWidget::move_calendar() {
  auto x_pos = window()->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = window()->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_calendar_widget->move(x_pos, y_pos + 1);
  m_calendar_widget->raise();
}

void DateInputWidget::set_default_style() {
  setStyleSheet(QString(R"(
    #dateinputwidget {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      font-family: Roboto;
      font-size: %3px;
    }

    #dateinputwidget:hover {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_width(1)).arg(scale_height(1)).arg(scale_height(12)));
}

void DateInputWidget::set_focus_style() {
  setStyleSheet(QString(R"(
    #dateinputwidget {
      background-color: #FFFFFF;
      border: %1px solid #4B23A0 %2px solid #4B23A0;
      font-family: Roboto;
      font-size: %3px;
    })").arg(scale_width(1)).arg(scale_height(1)).arg(scale_height(12)));
}

void DateInputWidget::update_label(date updated_date) {
  auto date = QDate(updated_date.year(), updated_date.month(),
    updated_date.day());
  setText(date.toString("MMM d, yyyy"));
}
