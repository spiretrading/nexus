#ifndef SPIRE_LOCAL_VALUE_MODEL_HPP
#define SPIRE_LOCAL_VALUE_MODEL_HPP
#include <utility>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

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
       * Constructs a model with an initial value.
       * @param value The initial value.
       */
      explicit LocalValueModel(Type value);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable typename UpdateSignal m_update_signal;
      Type m_value;
  };

  template<typename T>
  LocalValueModel<T>::LocalValueModel()
    : m_value() {}

  template<typename T>
  LocalValueModel<T>::LocalValueModel(Type value)
    : m_value(std::move(value)) {}

  template<typename T>
  const typename LocalValueModel<T>::Type&
      LocalValueModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State LocalValueModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State LocalValueModel<T>::set(const Type& value) {
    m_value = value;
    m_update_signal(value);
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection LocalValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }
}

#endif
