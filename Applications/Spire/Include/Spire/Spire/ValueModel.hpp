#ifndef SPIRE_VALUE_MODEL_HPP
#define SPIRE_VALUE_MODEL_HPP
#include <boost/signals2/connection.hpp>
#include <QValidator>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Base class used to model a single value.
   * @param <T> The type of value being modeled.
   */
  template<typename T>
  class ValueModel {
    public:

      /** The type of value being modeled. */
      using Type = T;

      /**
       * Signals a change to the current value.
       * @param current The current value.
       */
      using CurrentSignal = Signal<void (const Type&)>;

      virtual ~ValueModel() = default;

      /**
       * Returns the state of the current value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      virtual QValidator::State get_state() const;

      /** Returns the current value. */
      virtual const Type& get_current() const = 0;

      /**
       * Sets the current value. By default this operation is a no-op that
       * always returns <i>QValidator::State::Invalid</i>.
       */
      virtual QValidator::State set_current(const Type& value);

      /** Connects a slot to the CurrentSignal. */
      virtual boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an empty model. */
      ValueModel() = default;

    private:
      ValueModel(const ValueModel&) = delete;
      ValueModel& operator =(const ValueModel&) = delete;
  };

  template<typename T>
  QValidator::State ValueModel<T>::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State ValueModel<T>::set_current(const Type& value) {
    return QValidator::State::Invalid;
  }
}

#endif
