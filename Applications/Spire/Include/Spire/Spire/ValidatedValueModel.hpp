#ifndef SPIRE_VALIDATED_VALUE_MODEL_HPP
#define SPIRE_VALIDATED_VALUE_MODEL_HPP
#include <concepts>
#include <memory>
#include <utility>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Wraps a base ValueModel with a function that validates updates.
   * @param <T> The type of value to model.
   * @param <F> The type of validator to use.
   */
  template<typename T, std::invocable<const T&> F>
  class ValidatedValueModel : public ValueModel<T> {
    public:

      /** The type of value to model. */
      using Type = typename ValueModel<T>::Type;

      /** The type of validator to use. */
      using Validator = F;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a ValidatedValueModel using a uniquely owned LocalValueModel
       * as its base.
       * @param validator The function used to validate updates.
       */
      template<std::invocable<const Type&> V>
      explicit ValidatedValueModel(V&& validator)
        : ValidatedValueModel(std::forward<V>(validator),
            std::make_shared<LocalValueModel<Type>>()) {}

      /**
       * Constructs a ValidatedValueModel.
       * @param validator The function used to validate updates.
       * @param base The base ValueModel to apply the validation to.
       */
      template<std::invocable<const Type&> V>
      ValidatedValueModel(V&& validator, std::shared_ptr<ValueModel<T>> base)
        : m_validator(std::forward<V>(validator)),
          m_base(std::move(base)),
          m_state(QValidator::State::Acceptable),
          m_update_connection(m_base->connect_update_signal(m_update_signal)) {}

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      Validator m_validator;
      std::shared_ptr<ValueModel<Type>> m_base;
      QValidator::State m_state;
      boost::signals2::scoped_connection m_update_connection;
  };

  /**
   * Returns a newly constructed ValidatedValueModel using a uniquely owned
   * LocalValueModel as its base.
   * @param base The base ValueModel to apply the validation to.
   * @param validator The function used to validate updates.
   */
  template<typename T, std::invocable<const T&> V>
  auto make_validated_value_model(
      V&& validator, std::shared_ptr<ValueModel<T>> base) {
    return std::make_shared<ValidatedValueModel<T, std::remove_reference_t<V>>>(
      std::forward<V>(validator), std::move(base));
  }

  /**
   * Returns a newly constructed ValidatedValueModel using a uniquely owned
   * LocalValueModel as its base.
   * @param validator The function used to validate updates.
   */
  template<typename T, std::invocable<const T&> V>
  auto make_validated_value_model(V&& validator) {
    return std::make_shared<ValidatedValueModel<T, std::remove_reference_t<V>>>(
      std::forward<V>(validator));
  }

  template<typename T, std::invocable<const T&> F>
  QValidator::State ValidatedValueModel<T, F>::get_state() const {
    if(m_state == QValidator::State::Acceptable) {
      return m_base->get_state();
    }
    return m_state;
  }

  template<typename T, std::invocable<const T&> F>
  const typename ValidatedValueModel<T, F>::Type&
      ValidatedValueModel<T, F>::get() const {
    return m_base->get();
  }

  template<typename T, std::invocable<const T&> F>
  QValidator::State ValidatedValueModel<T, F>::test(const Type& value) const {
    auto validation = m_validator(value);
    if(validation == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    auto base_validation = m_base->test(value);
    if(base_validation == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    } else if(validation == QValidator::State::Intermediate ||
        base_validation == QValidator::State::Intermediate) {
      return QValidator::State::Intermediate;
    }
    return QValidator::State::Acceptable;
  }

  template<typename T, std::invocable<const T&> F>
  QValidator::State ValidatedValueModel<T, F>::set(const Type& value) {
    auto validation = m_validator(value);
    if(validation == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    auto base_validation = m_base->set(value);
    if(base_validation == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    } else if(validation == QValidator::State::Intermediate) {
      m_state = QValidator::State::Intermediate;
      return QValidator::State::Intermediate;
    } else if(base_validation == QValidator::State::Intermediate) {
      m_state = QValidator::State::Acceptable;
      return QValidator::State::Intermediate;
    }
    m_state = QValidator::State::Acceptable;
    return QValidator::State::Acceptable;
  }

  template<typename T, std::invocable<const T&> F>
  boost::signals2::connection ValidatedValueModel<T, F>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }
}

#endif
