#ifndef SPIRE_DATE_BOX_HPP
#define SPIRE_DATE_BOX_HPP
#include <QWidget>
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Specifies the accepted date format. */
  enum class DateFormat {

    /** The year, month, and day can be input in a YYYY-MM-DD format. */
    YYYYMMDD,

    /** The month and day can be input in a MM-DD format. */
    MMDD
  };
}

  /** Displays an input field for a date. */
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

      /** Returns <code>true</code> iff the DateBox is read-only. */
      bool is_read_only() const;

      /** Sets whether the DateBox is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<OptionalDateModel> m_model;
      FocusObserver m_focus_observer;
      IntegerBox* m_year_box;
      QWidget* m_year_dash;
      IntegerBox* m_month_box;
      IntegerBox* m_day_box;
      QWidget* m_body;
      OverlayPanel* m_date_picker;
      bool m_is_read_only;
      Styles::DateFormat m_format;
      boost::signals2::scoped_connection m_style_connection;

      void on_focus(FocusObserver::State state);
      void on_style();
  };
}

#endif
