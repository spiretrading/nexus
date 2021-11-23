#ifndef SPIRE_CONSTANT_VALUE_MODEL_HPP
#define SPIRE_CONSTANT_VALUE_MODEL_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ValueModel storing an immutable constant.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class ConstantValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using CurrentSignal = typename ValueModel<T>::CurrentSignal;

      /** Constructs a model using a default constructed value. */
      ConstantValueModel();

      /**
       * Constructs a model with an explicit constant.
       * @param current The constant to store.
       */
      ConstantValueModel(Type current);

      const Type& get() const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      Type m_current;
  };

  /**
   * Allocates a shared ConstantValueModel.
   * @param current The constant to store in the model.
   */
  template<typename T>
  auto make_constant_value_model(T&& current) {
    using Type = std::decay_t<T>;
    return std::make_shared<ConstantValueModel<Type>>(std::forward<T>(current));
  }

  template<typename T>
  ConstantValueModel<T>::ConstantValueModel()
    : m_current() {}

  template<typename T>
  ConstantValueModel<T>::ConstantValueModel(Type current)
    : m_current(std::move(current)) {}

  template<typename T>
  const typename ConstantValueModel<T>::Type&
      ConstantValueModel<T>::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State ConstantValueModel<T>::set(const Type& value) {
    return QValidator::State::Invalid;
  }

  template<typename T>
  boost::signals2::connection ConstantValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return {};
  }
}

#endif
