#ifndef SPIRE_DURATION_BOX_HPP
#define SPIRE_DURATION_BOX_HPP
#include <memory>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"

namespace Spire {
  class Box;

namespace Styles {

  /** Selects the colon field. */
  using Colon = StateSelector<void, struct DurationBoxColonTag>;
}

  /** A ScalarValueModel over a boost::posix_time::time_duration. */
  using DurationModel = ScalarValueModel<boost::posix_time::time_duration>;

  /** A ScalarValueModel over an optional boost::posix_time::time_duration. */
  using OptionalDurationModel =
    ScalarValueModel<boost::optional<boost::posix_time::time_duration>>;

  /** A LocalScalarValueModel over a boost::posix_time::time_duration. */
  using LocalDurationModel =
    LocalScalarValueModel<boost::posix_time::time_duration>;

  /**
   * A LocalScalarValueModel over an optional boost::posix_time::time_duration.
   */
  using LocalOptionalDurationModel =
    LocalScalarValueModel<boost::optional<boost::posix_time::time_duration>>;

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
      explicit DurationBox(std::shared_ptr<OptionalDurationModel> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OptionalDurationModel>& get_current() const;

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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<OptionalDurationModel> m_current;
      boost::optional<boost::posix_time::time_duration> m_submission;
      IntegerBox* m_hour_field;
      IntegerBox* m_minute_field;
      DecimalBox* m_second_field;
      Box* m_input_box;
      bool m_is_read_only;
      bool m_is_rejected;
      bool m_has_update;
      boost::signals2::scoped_connection m_style_connection;

      void on_current(
        const boost::optional<boost::posix_time::time_duration>& current);
      void on_submit();
      void on_reject();
      void on_style();
      void update_empty_fields();
  };

  /**
   * Returns a newly constructed time box.
   * @param time The time displayed in the time box.
   * @param parent The parent widget.
   */
  DurationBox* make_time_box(
    const boost::optional<boost::posix_time::time_duration>& time,
    QWidget* parent = nullptr);

  /**
   * Returns a newly constructed time box without a default time.
   * @param parent The parent widget.
   */
  DurationBox* make_time_box(QWidget* parent = nullptr);

  /** Returns a model which represents a time of day. */
  std::shared_ptr<OptionalDurationModel> make_time_of_day_model();

  /**
   * Returns a model which represents a time of day.
   * @param time The current time of day.
   */
  std::shared_ptr<OptionalDurationModel> make_time_of_day_model(
    const boost::optional<boost::posix_time::time_duration>& time);
}

#endif
