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

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /** Constructs a model using a default constructed value. */
      ConstantValueModel();

      /**
       * Constructs a model with an explicit constant.
       * @param value The constant to store.
       */
      ConstantValueModel(Type value);

      const Type& get() const override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      Type m_value;
  };

  /**
   * Allocates a shared ConstantValueModel.
   * @param value The constant to store in the model.
   */
  template<typename T>
  auto make_constant_value_model(T&& value) {
    using Type = std::decay_t<T>;
    return std::make_shared<ConstantValueModel<Type>>(std::forward<T>(value));
  }

  template<typename T>
  ConstantValueModel<T>::ConstantValueModel()
    : m_value() {}

  template<typename T>
  ConstantValueModel<T>::ConstantValueModel(Type value)
    : m_value(std::move(value)) {}

  template<typename T>
  const typename ConstantValueModel<T>::Type&
      ConstantValueModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  boost::signals2::connection ConstantValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return {};
  }
}

#endif
