#ifndef SPIRE_DURATION_BOX_HPP
#define SPIRE_DURATION_BOX_HPP
#include <memory>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the colon field. */
  using Colon = StateSelector<void, struct DurationBoxColonTag>;
}

  /** Displays a widget for inputting a time duration. */
  class DurationBox : public QWidget {
    public:

      /**
       * Signals the current value is being submitted.
       * @param submission The value being submitted.
       */
      using SubmitSignal = Signal<void (
        const boost::optional<boost::posix_time::time_duration>& submission)>;

      /**
       * Signals the current value was rejected as a submission.
       * @param rejection The value that was rejected.
       */
      using RejectSignal = Signal<void (
        const boost::optional<boost::posix_time::time_duration>& rejection)>;

      /**
       * Constructs a DurationBox using a local model.
       * @param parent The parent widget.
       */
      explicit DurationBox(QWidget* parent = nullptr);

      /**
       * Constructs a DurationBox.
       * @param model The model used for the current value.
       * @param parent The parent widget.
       */
      explicit DurationBox(std::shared_ptr<OptionalDurationModel> model,
        QWidget* parent = nullptr);

      /** Returns the model used for the current value. */
      const std::shared_ptr<OptionalDurationModel>& get_model() const;

      //! Returns <code>true</code> iff this box is read-only.
      bool is_read_only() const;

      //! Sets whether the box is read-only.
      void set_read_only(bool is_read_only);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<OptionalDurationModel> m_model;
      boost::optional<boost::posix_time::time_duration> m_submission;
      IntegerBox* m_hour_field;
      IntegerBox* m_minute_field;
      DecimalBox* m_second_field;
      bool m_is_read_only;

      void on_submit();
      void on_reject();
  };
}

#endif
