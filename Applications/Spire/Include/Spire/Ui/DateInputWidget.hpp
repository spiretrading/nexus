#ifndef SPIRE_DATE_INPUT_WIDGET_HPP
#define SPIRE_DATE_INPUT_WIDGET_HPP
#include <QLabel>
#include "Spire/Ui/CalendarWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a date selection widget with a pop-up calendar.
  class DateInputWidget : public QLabel {
    public:

      //! Signal type for date selection changes.
      /*!
        \param date The selected date.
      */
      using SelectedSignal = Signal<void (boost::posix_time::ptime date)>;

      //! Constructs a DateInputWidget with an initially selected date.
      /*
        \param initial_date The initially selected date.
        \param parent The parent widget.
      */
      explicit DateInputWidget(const boost::posix_time::ptime& initial_date,
        QWidget* parent = nullptr);

      //! Connects a slot to the date selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void moveEvent(QMoveEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      CalendarWidget* m_calendar_widget;
      DropShadow* m_drop_shadow;

      void move_calendar();
      void set_default_style();
      void set_focus_style();
      void update_label(boost::gregorian::date update_date);
  };
}

#endif
