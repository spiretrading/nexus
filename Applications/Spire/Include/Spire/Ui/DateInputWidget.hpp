#ifndef SPIRE_DATE_INPUT_WIDGET_HPP
#define SPIRE_DATE_INPUT_WIDGET_HPP
#include <QWidget>
#include "Spire/Ui/CalendarWidget.hpp"

namespace Spire {

  class DateInputWidget : public QWidget {
    public:

      DateInputWidget(boost::posix_time::ptime date,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      CalendarWidget* m_calendar_widget;

      void move_calendar();
  };
}

#endif
