#ifndef SPIRE_DURATION_BOX_HPP
#define SPIRE_DURATION_BOX_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/LocalDurationModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for inputting a time duration.
  class DurationBox : public Styles::StyledWidget {
    public:

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const Duration& value)>;

      //! Signals that submission value has changed.
      /*!
        \param value The submission value.
      */
      using SubmitSignal = Signal<void (const Duration& value)>;

      //! Constructs a DurationBox.
      /*!
        \param parent The parent widget.
      */
      explicit DurationBox(QWidget* parent = nullptr);

      //! Constructs a DurationBox.
      /*!
        \param model The model used for the current value.
        \param parent The parent widget.
      */
      explicit DurationBox(std::shared_ptr<LocalDurationModel> model,
        QWidget* parent = nullptr);

      //! Returns the current value model.
      const std::shared_ptr<LocalDurationModel>& get_model() const;

      //! Returns whether a warning is displayed when a submission is rejected.
      bool is_warning_displayed() const;

      //! Sets whether a warning is displayed when a submission is rejected.
      /*!
        \param is_displayed True iff the warning style should be displayed.
      */
      void set_warning_displayed(bool is_displayed);

      //! Connects a slot to the RejectSignal.
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

       //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    private:
      mutable RejectSignal m_reject_signal;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<LocalDurationModel> m_model;
      Duration m_submission;
      Box* m_box;
      IntegerBox* m_hour_field;
      IntegerBox* m_minute_field;
      DecimalBox* m_second_field;
      bool m_is_warning_displayed;

      void on_submit();
      void on_reject();
      void clear_leading_trailing_zeros();
      void set_leading_trailing_zeros();
  };
}

#endif
