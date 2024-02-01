#ifndef SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#define SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ScalarValueModel by storing and updating a local value.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalScalarValueModel : public ScalarValueModel<T> {
    public:
      using Type = typename ScalarValueModel<T>::Type;

      using Scalar = typename ScalarValueModel<T>::Scalar;

      using UpdateSignal = typename ScalarValueModel<T>::UpdateSignal;

      /** Constructs a default model. */
      LocalScalarValueModel();

      /**
       * Constructs a model with an initial value.
       * @param value The initial value.
       */
      LocalScalarValueModel(Type value);

      /** Sets the minimum assignable value. */
      void set_minimum(const boost::optional<Scalar>& minimum);

      /** Sets the maximum assignable value. */
      void set_maximum(const boost::optional<Scalar>& maximum);

      /** Sets the increment. */
      void set_increment(const boost::optional<Scalar>& increment);

      boost::optional<Scalar> get_minimum() const override;

      boost::optional<Scalar> get_maximum() const override;

      boost::optional<Scalar> get_increment() const override;

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      ScalarValueModelDecorator<Type> m_model;
  };

  template<typename T>
  LocalScalarValueModel<T>::LocalScalarValueModel()
    : m_model(std::make_shared<LocalValueModel<Type>>()) {}

  template<typename T>
  LocalScalarValueModel<T>::LocalScalarValueModel(Type value)
    : m_model(std::make_shared<LocalValueModel<Type>>(std::move(value))) {}

  template<typename T>
  void LocalScalarValueModel<T>::set_minimum(
      const boost::optional<Scalar>& minimum) {
    m_model.set_minimum(minimum);
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_maximum(
      const boost::optional<Scalar>& maximum) {
    m_model.set_maximum(maximum);
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_increment(
      const boost::optional<Scalar>& increment) {
    m_model.set_increment(increment);
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Scalar>
      LocalScalarValueModel<T>::get_minimum() const {
    return m_model.get_minimum();
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Scalar>
      LocalScalarValueModel<T>::get_maximum() const {
    return m_model.get_maximum();
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Scalar>
      LocalScalarValueModel<T>::get_increment() const {
    return m_model.get_increment();
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::get_state() const {
    return m_model.get_state();
  }

  template<typename T>
  const typename LocalScalarValueModel<T>::Type&
      LocalScalarValueModel<T>::get() const {
    return m_model.get();
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::test(const Type& value) const {
    return m_model.test(value);
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::set(const Type& value) {
    return m_model.set(value);
  }

  template<typename T>
  boost::signals2::connection LocalScalarValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_model.connect_update_signal(slot);
  }
}

#endif
