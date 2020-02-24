#ifndef SPIRE_MONTH_AND_YEAR_SPIN_BOX_HPP
#define SPIRE_MONTH_AND_YEAR_SPIN_BOX_HPP
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a horizontal spin box that cycles through months and years.
  class MonthAndYearSpinBox : public QWidget {
    public:

      //! Signals that a month has been selected.
      using MonthSignal = Signal<void (boost::gregorian::date)>;

      //! Constructs a MonthAndYearSpinBox with an initial month and year.
      /*
        \param initial_date The initial month and year to display.
        \param parent The parent widget.
      */
      MonthAndYearSpinBox(boost::gregorian::date initial_date,
        QWidget* parent = nullptr);

      //! Returns the current month and year.
      boost::gregorian::date get_date() const;

      //! Sets the current month and year.
      void set_date(boost::gregorian::date date);

      //! Connects a slot to the month signal.
      boost::signals2::connection connect_month_signal(
        const MonthSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable MonthSignal m_month_signal;
      boost::gregorian::date m_date;
      QLabel* m_month_label;
      QLabel* m_left_label;
      QLabel* m_right_label;

      void update_label();
  };
}

#endif
