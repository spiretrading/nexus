#ifndef SPIRE_DATE_INPUT_WIDGET_HPP
#define SPIRE_DATE_INPUT_WIDGET_HPP
#include <QLabel>
#include "Spire/Ui/CalendarWidget.hpp"

namespace Spire {

  //! Displays a date selection widget with a pop-up calendar.
  class DateInputWidget : public QLabel {
    public:

      //! Constructs a DateInputWidget with an initially selected date.
      /*
        \param initial_date The initially selected date.
        \param parent The parent widget.
      */
      explicit DateInputWidget(const boost::posix_time::ptime& initial_date,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void moveEvent(QMoveEvent* event) override;

    private:
      CalendarWidget* m_calendar_widget;

      void move_calendar();
      void set_default_style();
      void set_focus_style();
      void update_label(boost::gregorian::date update_date);
  };
}

#endif
