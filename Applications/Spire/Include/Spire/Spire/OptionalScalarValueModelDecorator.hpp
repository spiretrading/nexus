#ifndef SPIRE_OPTIONAL_SCALAR_VALUE_MODEL_DECORATOR_HPP
#define SPIRE_OPTIONAL_SCALAR_VALUE_MODEL_DECORATOR_HPP
#include <functional>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Takes a ScalarValueModel and decorates it to work with an optional
   * ScalarValueModel.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class OptionalScalarValueModelDecorator :
      public ScalarValueModel<boost::optional<T>> {
    public:
      using Type = ScalarValueModel<boost::optional<T>>::Type;
      using Scalar = ScalarValueModel<boost::optional<T>>::Scalar;
      using UpdateSignal = ScalarValueModel<boost::optional<T>>::UpdateSignal;

      /**
       * Constructs an OptionalScalarValueModelDecorator.
       * @param model The model to decorate.
       */
      explicit OptionalScalarValueModelDecorator(
        std::shared_ptr<ScalarValueModel<Scalar>> model);

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

      boost::optional<Scalar> get_minimum() const override;

      boost::optional<Scalar> get_maximum() const override;

      boost::optional<Scalar> get_increment() const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ScalarValueModel<Scalar>> m_model;
      boost::optional<Scalar> m_current;
      boost::signals2::scoped_connection m_connection;

      void on_update(const Type& value);
  };

  template<typename T>
  OptionalScalarValueModelDecorator(std::shared_ptr<T>) ->
    OptionalScalarValueModelDecorator<typename T::Type>;

  /**
   * Constructs an OptionalScalarValueModelDecorator.
   * @param model The model to decorate.
   */
  template<typename T>
  auto make_optional_scalar_value_model_decorator(std::shared_ptr<T> model) {
    return std::make_shared<
      OptionalScalarValueModelDecorator<typename T::Type>>(std::move(model));
  }

  template<typename T>
  OptionalScalarValueModelDecorator<T>::OptionalScalarValueModelDecorator(
    std::shared_ptr<ScalarValueModel<Scalar>> model)
    : m_model(std::move(model)),
      m_current(m_model->get()),
      m_connection(m_model->connect_update_signal(std::bind_front(
        &OptionalScalarValueModelDecorator::on_update, this))) {}

  template<typename T>
  QValidator::State OptionalScalarValueModelDecorator<T>::get_state() const {
    return m_model->get_state();
  }

  template<typename T>
  const typename OptionalScalarValueModelDecorator<T>::Type&
      OptionalScalarValueModelDecorator<T>::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State OptionalScalarValueModelDecorator<T>::test(
      const Type& value) const {
    if(!value) {
      return QValidator::State::Invalid;
    }
    return m_model->test(*value);
  }

  template<typename T>
  QValidator::State OptionalScalarValueModelDecorator<T>::set(
      const Type& value) {
    if(!value) {
      return QValidator::State::Invalid;
    }
    return m_model->set(*value);
  }

  template<typename T>
  boost::signals2::connection
      OptionalScalarValueModelDecorator<T>::connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  boost::optional<typename OptionalScalarValueModelDecorator<T>::Scalar>
      OptionalScalarValueModelDecorator<T>::get_minimum() const {
    return m_model->get_minimum();
  }

  template<typename T>
  boost::optional<typename OptionalScalarValueModelDecorator<T>::Scalar>
      OptionalScalarValueModelDecorator<T>::get_maximum() const {
    return m_model->get_maximum();
  }

  template<typename T>
  boost::optional<typename OptionalScalarValueModelDecorator<T>::Scalar>
      OptionalScalarValueModelDecorator<T>::get_increment() const {
    return m_model->get_increment();
  }

  template<typename T>
  void OptionalScalarValueModelDecorator<T>::on_update(const Type& value) {
    m_current = value;
    m_update_signal(m_current);
  }
}

#endif
