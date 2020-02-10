#ifndef SPIRE_CALENDAR_WIDGET_HPP
#define SPIRE_CALENDAR_WIDGET_HPP
#include <QCalendarWidget>

namespace Spire {

  class CalendarWidget : public QCalendarWidget {
    public:

      explicit CalendarWidget(QWidget* parent = nullptr);
  };
}

#endif
