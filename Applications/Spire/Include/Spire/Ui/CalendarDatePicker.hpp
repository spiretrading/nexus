#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QWidget>
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"

namespace Spire {
namespace Styles {

  /** Selects today. */
  using Today = StateSelector<void, struct TodayTag>;

  /** Selects a day outside of the current month. */
  using OutOfMonth = StateSelector<void, struct OutOfMonthTag>;
}

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
      CalendarDatePicker(QWidget* parent = nullptr);

      /**
       * Constructs a CalendarDatePicker using a LocalOptionalDateModel.
       * @param current The current date.
       * @param parent The parent widget.
       */
      CalendarDatePicker(boost::gregorian::date current,
        QWidget* parent = nullptr);

      /**
       * Constructs a CalendarDatePicker.
       * @param model The current value's model.
       * @param parent The parent widget.
       */
      CalendarDatePicker(std::shared_ptr<OptionalDateModel> model,
        QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<OptionalDateModel>& get_model() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      class MonthSelector;
      class CalendarDayLabel;
      std::shared_ptr<OptionalDateModel> m_model;
      MonthSelector* m_month_selector;
      QWidget* m_day_header;
      ListView* m_calendar_view;
      std::vector<CalendarDayLabel*> m_day_labels;
      std::shared_ptr<ArrayListModel> m_calendar_model;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_list_current_connection;

      int get_index(boost::gregorian::date day) const;
      void populate_calendar(const std::function<
        void (int index, boost::gregorian::date day)> assign);
      void set_current_index(const boost::optional<int>& index);
      void update_calendar_model();
      void on_current(const boost::optional<boost::gregorian::date>& day);
      void on_current_month(boost::gregorian::date month);
      void on_list_current(const boost::optional<int> index);
      void on_submit(boost::gregorian::date day) const;
  };
}

#endif
