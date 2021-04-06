#ifndef SPIRE_LOCAL_DURATION_MODEL_HPP
#define SPIRE_LOCAL_DURATION_MODEL_HPP
#include "boost/date_time/posix_time/posix_time.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a LocalDurationModel by storing and updating a time duration.
   */
  class LocalDurationModel :
      public LocalValueModel<boost::posix_time::time_duration> {
    public:

      using CurrentSignal = typename
        LocalValueModel<boost::posix_time::time_duration>::CurrentSignal;

      /** Constructs a default model. */
      LocalDurationModel();

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalDurationModel(boost::posix_time::time_duration current);

      /** Sets the minimum value assignable to current. */
      void set_minimum(
        const boost::optional<boost::posix_time::time_duration>& minimum);

      /** Sets the maximum value assignable to current. */
      void set_maximum(
        const boost::optional<boost::posix_time::time_duration>& maximum);

      /** Returns the minimum duration or none if there is no minimum. */
      boost::optional<boost::posix_time::time_duration> get_minimum() const;

      /** Returns the maximum duration or none if there is no maximum. */
      boost::optional<boost::posix_time::time_duration> get_maximum() const;

      /**
       * Returns the state of the current value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      QValidator::State get_state() const;

      /** Returns the current value. */
      const boost::posix_time::time_duration& get_current() const;

      /**
       * Sets the current value. By default this operation is a no-op that
       * always returns <i>QValidator::State::Invalid</i>.
       */
      QValidator::State set_current(
        const boost::posix_time::time_duration& value) override;

      //! Returns the hour model.
      const std::shared_ptr<LocalIntegerModel>& get_hour_model() const;

      //! Returns the minute model.
      const std::shared_ptr<LocalIntegerModel>& get_minute_model() const;

      //! Returns the second model.
      const std::shared_ptr<LocalScalarValueModel<DecimalBox::Decimal>>&
        get_second_model() const;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      QValidator::State m_state;
      boost::optional<boost::posix_time::time_duration> m_minimum;
      boost::optional<boost::posix_time::time_duration> m_maximum;
      std::shared_ptr<LocalIntegerModel> m_hour_model;
      std::shared_ptr<LocalIntegerModel> m_minute_model;
      std::shared_ptr<LocalScalarValueModel<DecimalBox::Decimal>> m_second_model;
  };
}

#endif
