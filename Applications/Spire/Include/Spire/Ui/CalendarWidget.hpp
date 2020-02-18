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
#include "Spire/Ui/CalendarDayWidget.hpp"
#include "Spire/Ui/CalendarModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/MonthAndYearSpinBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class CalendarWidget : public QWidget {
    public:

      using DateSignal = Signal<void (const boost::gregorian::date&)>;

      explicit CalendarWidget(const boost::gregorian::date& selected_date,
        QWidget* parent = nullptr);

      void set_date(const boost::gregorian::date& date);

      boost::signals2::connection connect_date_signal(
          const DateSignal::slot_type& slot) const {
        return m_date_signal.connect(slot);
      }

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
