#ifndef SPIRE_LOCAL_VALUE_MODEL_HPP
#define SPIRE_LOCAL_VALUE_MODEL_HPP
#include <utility>
#include <boost/signals2/connection.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel by storing and updating a local value.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using CurrentSignal = typename ValueModel<T>::CurrentSignal;

      /** Constructs a default model. */
      LocalValueModel() = default;

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalValueModel(Type current);

      const Type& get_current() const override;

      QValidator::State set_current(const Type& value);

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const;

    private:
      mutable typename CurrentSignal m_current_signal;
      Type m_current;
  };

  template<typename T>
  LocalValueModel<T>::LocalValueModel(Type current)
    : m_current(std::move(current)) {}

  template<typename T>
  const typename LocalValueModel<T>::Type&
      LocalValueModel<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  QValidator::State LocalValueModel<T>::set_current(const Type& value) {
    m_current = value;
    m_current_signal(m_current);
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection LocalValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }
}

#endif
