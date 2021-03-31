#ifndef SPIRE_LOCAL_DURATION_MODEL_HPP
#define SPIRE_LOCAL_DURATION_MODEL_HPP
#include "boost/date_time/posix_time/posix_time.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents the time duration.
  using Duration = boost::posix_time::time_duration;

  /**
   * Implements a LocalDurationModel by storing and updating a time duration.
   */
  class LocalDurationModel : public LocalValueModel<Duration> {
    public:

      using CurrentSignal = typename LocalValueModel<Duration>::CurrentSignal;

      /** Constructs a default model. */
      LocalDurationModel();

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalDurationModel(const Duration& current);

      /** Sets the minimum value assignable to current. */
      void set_minimum(const boost::optional<Duration>& minimum);

      /** Sets the maximum value assignable to current. */
      void set_maximum(const boost::optional<Duration>& maximum);

      /** Returns the minimum duration or none if there is no minimum. */
      boost::optional<Duration> get_minimum() const;

      /** Returns the maximum duration or none if there is no maximum. */
      boost::optional<Duration> get_maximum() const;

      /**
       * Returns the state of the current value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      QValidator::State get_state() const;

      /** Returns the current value. */
      const Duration& get_current() const;

      /**
       * Sets the current value. By default this operation is a no-op that
       * always returns <i>QValidator::State::Invalid</i>.
       */
      QValidator::State set_current(const Duration& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      QValidator::State m_state;
      boost::optional<Duration> m_minimum;
      boost::optional<Duration> m_maximum;
  };
}

#endif
