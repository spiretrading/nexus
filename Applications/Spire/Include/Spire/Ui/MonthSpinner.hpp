#ifndef SPIRE_MONTH_SPINNER_HPP
#define SPIRE_MONTH_SPINNER_HPP
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a date. */
  using DateModel = ValueModel<boost::gregorian::date>;

  /** A LocalValueModel over a date. */
  using LocalDateModel = LocalValueModel<boost::gregorian::date>;

  /** Represents an input for selecting a month and year. */
  class MonthSpinner : public QWidget {
    public:

      /**
       * Constructs a MonthSpinner using a LocalDateModel and the system's
       * current date as the initial date.
       * @param parent The parent widget.
       */
      explicit MonthSpinner(QWidget* parent = nullptr);

      /**
       * Constructs a MonthSpinner using a LocalDateModel and an initial date.
       * @param date The initial date.
       * @param parent The parent widget.
       */
      explicit MonthSpinner(
        boost::gregorian::date date, QWidget* parent = nullptr);

      /**
       * Constructs a MonthSpinner.
       * @param model The DateModel.
       * @param parent The parent widget.
       */
      explicit MonthSpinner(
        std::shared_ptr<DateModel> model, QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<DateModel>& get_model() const;

    private:
      std::shared_ptr<DateModel> m_model;
      TextBox* m_label;
      Button* m_previous_button;
      Button* m_next_button;

    void decrement();
    void increment();
    void update_label();
    void on_current(boost::gregorian::date date);
  };
}

#endif
