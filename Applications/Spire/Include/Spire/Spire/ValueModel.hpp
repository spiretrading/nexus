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
       * Signals a change to the value.
       * @param value The updated value.
       */
      using UpdateSignal = Signal<void (const Type& value)>;

      virtual ~ValueModel() = default;

      /**
       * Returns the state of the value, by default this is
       * <i>QValidator::State::Acceptable</i>
       */
      virtual QValidator::State get_state() const;

      /** Returns the value. */
      virtual const Type& get() const = 0;

      /**
       * Sets the value. By default this operation is a no-op that always
       * returns <i>QValidator::State::Invalid</i>.
       */
      virtual QValidator::State set(const Type& value);

      /** Connects a slot to the UpdateSignal. */
      virtual boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const = 0;

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
  QValidator::State ValueModel<T>::set(const Type& value) {
    return QValidator::State::Invalid;
  }
}

#endif
