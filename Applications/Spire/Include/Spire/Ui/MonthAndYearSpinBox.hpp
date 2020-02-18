#ifndef SPIRE_MONTH_AND_YEAR_SPIN_BOX_HPP
#define SPIRE_MONTH_AND_YEAR_SPIN_BOX_HPP
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class MonthAndYearSpinBox : public QWidget {
    public:

      using MonthSignal = Signal<void (boost::gregorian::date)>;

      MonthAndYearSpinBox(boost::gregorian::date initial_date,
        QWidget* parent = nullptr);

      boost::gregorian::date get_date() const;

      void set_date(boost::gregorian::date date);

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
