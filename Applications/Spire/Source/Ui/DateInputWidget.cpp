#include "Spire/Ui/DateInputWidget.hpp"
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::posix_time;
using namespace Spire;

DateInputWidget::DateInputWidget(ptime date, QWidget* parent)
    : QWidget(parent) {
  m_calendar_widget = new CalendarWidget(date.date(), this);
  m_calendar_widget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  m_calendar_widget->setAttribute(Qt::WA_ShowWithoutActivating);
  auto shadow = std::make_unique<DropShadow>(false, m_calendar_widget);
  m_calendar_widget->hide();
  window()->installEventFilter(this);
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
  }
  return QWidget::eventFilter(watched, event);
}

void DateInputWidget::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_calendar_widget->isVisible()) {
      m_calendar_widget->hide();
    } else {
      move_calendar();
      m_calendar_widget->show();
    }
  }
}

void DateInputWidget::move_calendar() {
  auto x_pos = window()->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = window()->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_calendar_widget->move(x_pos, y_pos + 1);
  m_calendar_widget->raise();
}
