#ifndef SPIRE_CALENDAR_WIDGET_HPP
#define SPIRE_CALENDAR_WIDGET_HPP
#include <array>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Scalar.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/CalendarModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a calendar month for selecting dates.
  class CalendarWidget : public QWidget {
    public:

      //! Signals that a date was selected.
      using DateSignal = Signal<void (boost::gregorian::date)>;

      //! Constructs a CalendarWidget with a selected date.
      /*
        \param selected_date The default selected date.
        \param parent The parent widget.
      */
      explicit CalendarWidget(boost::gregorian::date selected_date,
        QWidget* parent = nullptr);

      //! Sets the selected date.
      void set_date(const boost::gregorian::date& date);

      //! Connects a slot to the date signal.
      boost::signals2::connection connect_date_signal(
        const DateSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable DateSignal m_date_signal;
      MonthAndYearSpinBox* m_month_spin_box;
      boost::gregorian::date m_selected_date;
      CalendarDayWidget* m_selected_date_widget;
      CalendarModel m_calendar_model;
      QGridLayout* m_calendar_layout;

      void on_date_selected(const boost::gregorian::date& date);
      void on_month_changed(const boost::gregorian::date& date);
      void add_day_label(QLayout* layout, const QString& text);
      CalendarDayWidget* get_day_widget(const boost::gregorian::date& date);
      void set_highlight();
      void update_calendar(const boost::gregorian::date& displayed_date);
  };
}

#endif
