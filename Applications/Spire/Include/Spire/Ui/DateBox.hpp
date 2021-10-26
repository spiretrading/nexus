#ifndef SPIRE_DATE_BOX_HPP
#define SPIRE_DATE_BOX_HPP
#include <QWidget>
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/IntegerBox.hpp"

namespace Spire {
namespace Styles {

  /** Specifies the accepted date format. */
  enum class DateFormat {

    /** The month and day can be input in a MM-DD format. */
    MMDD,

    /** The year, month, and day can be input in a YYYY-MM-DD format. */
    YYYYMMDD
  };
}

  class DateBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted date.
       */
      using SubmitSignal = Signal<
        void (const boost::optional<boost::gregorian::date>& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The rejected value.
       */
      using RejectSignal = Signal<
        void (const boost::optional<boost::gregorian::date>& value)>;

      explicit DateBox(const boost::optional<boost::gregorian::date>& current,
        QWidget* parent = nullptr);

      explicit DateBox(
        std::shared_ptr<OptionalDateModel> model, QWidget* parent = nullptr);

      const std::shared_ptr<OptionalDateModel>& get_model() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      FocusObserver m_focus_observer;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<OptionalDateModel> m_model;
      boost::signals2::scoped_connection m_current_connection;
      boost::optional<boost::gregorian::date> m_submission;
      bool m_is_modified;
      bool m_is_rejected;
      IntegerBox* m_year_field;
      boost::signals2::scoped_connection m_year_connection;
      TextBox* m_year_dash;
      IntegerBox* m_month_field;
      boost::signals2::scoped_connection m_month_connection;
      IntegerBox* m_day_field;
      boost::signals2::scoped_connection m_day_connection;
      OverlayPanel* m_panel;

      boost::optional<boost::gregorian::date> get_current() const;
      void populate_input_fields();
      void on_current(const boost::optional<boost::gregorian::date>& current);
      void on_field_current();
      void on_focus(FocusObserver::State state);
      void on_reject();
      void on_submit();
  };
}

#endif
