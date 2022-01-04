#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects today. */
  using Today = StateSelector<void, struct TodayTag>;

  /** Selects a day outside of the current month. */
  using OutOfMonth = StateSelector<void, struct OutOfMonthTag>;
}

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
      explicit CalendarDatePicker(
        boost::gregorian::date current, QWidget* parent = nullptr);

      /**
       * Constructs a CalendarDatePicker.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit CalendarDatePicker(
        std::shared_ptr<OptionalDateModel> current, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalDateModel>& get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      class MonthSpinner;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<OptionalDateModel> m_current;
      boost::signals2::scoped_connection m_current_connection;
      MonthSpinner* m_month_spinner;
      std::shared_ptr<ListModel<boost::gregorian::date>> m_calendar_model;
      ListView* m_calendar_view;
      boost::signals2::scoped_connection m_list_current_connection;

      boost::optional<int> get_index(boost::gregorian::date day) const;
      void set_current_index(const boost::optional<int>& index);
      void on_current(const boost::optional<boost::gregorian::date>& current);
      void on_current_month(boost::gregorian::date month);
      void on_list_current(const boost::optional<int>& index);
      void on_submit(boost::gregorian::date day);
  };
}

#endif
