#ifndef SPIRE_DATE_INPUT_WIDGET_HPP
#define SPIRE_DATE_INPUT_WIDGET_HPP
#include <QLabel>
#include "Spire/Ui/CalendarWidget.hpp"

namespace Spire {

  class DateInputWidget : public QLabel {
    public:

      DateInputWidget(boost::posix_time::ptime date,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      CalendarWidget* m_calendar_widget;

      void move_calendar();
      void set_default_style();
      void set_focus_style();
      void update_label(boost::gregorian::date update_date);
  };
}

#endif
