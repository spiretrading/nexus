#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a date. */
  using DateModel = ValueModel<boost::gregorian::date>;

  /** A LocalValueModel over a date. */
  using LocalDateModel = LocalValueModel<boost::gregorian::date>;

  /** A ScalarValueModel over an optional date. */
  using OptionalDateModel =
    ScalarValueModel<boost::optional<boost::gregorian::date>>;

  /** A LocalScalarValueModel over an optional date. */
  using LocalOptionalDateModel =
    LocalScalarValueModel<boost::optional<boost::gregorian::date>>;

  /** Displays a calendar for selecting dates. */
  class CalendarDatePicker : public QWidget {
    public:

      /**
       * Signals that a date was submitted.
       * @param date The submitted date.
       */
      using SubmitSignal = Signal<void (boost::gregorian::date date)>;

      /**
       * Constructs a CalendarDatePicker using a LocalOptionalDateModel. The
       * current date defaults to today's local date.
       * @param parent The parent widget.
       */
      explicit CalendarDatePicker(QWidget* parent = nullptr);

      /**
       * Constructs a CalendarDatePicker using a LocalOptionalDateModel.
       * @param current The current date.
       * @param parent The parent widget.
       */
      explicit CalendarDatePicker(boost::gregorian::date current,
        QWidget* parent = nullptr);

      /**
       * Constructs a CalendarDatePicker.
       * @param model The current value's model.
       * @param parent The parent widget.
       */
      explicit CalendarDatePicker(std::shared_ptr<OptionalDateModel> model,
        QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<OptionalDateModel>& get_model() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      class MonthSpinner;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<OptionalDateModel> m_model;
      MonthSpinner* m_month_spinner;

      void on_current(const boost::optional<boost::gregorian::date>& current);
  };
}

#endif