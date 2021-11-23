#ifndef SPIRE_LOCAL_VALUE_MODEL_HPP
#define SPIRE_LOCAL_VALUE_MODEL_HPP
#include <utility>
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ValueModel by storing and updating a local value.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /** Constructs a default model. */
      LocalValueModel();

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalValueModel(Type current);

      const Type& get() const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable typename UpdateSignal m_current_signal;
      Type m_current;
  };

  template<typename T>
  LocalValueModel<T>::LocalValueModel()
    : m_current() {}

  template<typename T>
  LocalValueModel<T>::LocalValueModel(Type current)
    : m_current(std::move(current)) {}

  template<typename T>
  const typename LocalValueModel<T>::Type&
      LocalValueModel<T>::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State LocalValueModel<T>::set(const Type& value) {
    m_current = value;
    m_current_signal(value);
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection LocalValueModel<T>::connect_current_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }
}

#endif
