#ifndef SPIRE_PROXY_SCALAR_VALUE_MODEL_HPP
#define SPIRE_PROXY_SCALAR_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ScalarValueModel.hpp"

namespace Spire {

  /**
   * Implements a ScalarValueModel that forwards calls to an underlying
   * ScalarValueModel.
   * @param <T> The type being proxied.
   */
  template<typename T>
  class ProxyScalarValueModel : public ScalarValueModel<T> {
    public:
      using Type = typename ScalarValueModel<T>::Type;
      using Scalar = typename ScalarValueModel<T>::Scalar;
      using UpdateSignal = typename ScalarValueModel<T>::UpdateSignal;

      /**
       * Constructs a ProxyScalarValueModel.
       * @param source The model to forward calls to.
       */
      explicit ProxyScalarValueModel(
        std::shared_ptr<ScalarValueModel<Type>> source);

      /** Returns the model being proxied. */
      std::shared_ptr<ScalarValueModel<Type>> get_source() const;

      /** Sets the model to forward calls to. */
      void set_source(std::shared_ptr<ScalarValueModel<Type>> source);

      boost::optional<Scalar> get_minimum() const override;

      boost::optional<Scalar> get_maximum() const override;

      boost::optional<Scalar> get_increment() const override;

      QValidator::State get_state() const override;

      const Type& get() const;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const;

    private:
      ProxyValueModel<Type> m_source;
  };

  /**
   * Constructs a ProxyScalarValueModel.
   * @param source The model to forward calls to.
   */
  template<typename T>
  auto make_proxy_scalar_value_model(
      std::shared_ptr<ScalarValueModel<T>> source) {
    return std::make_shared<ProxyScalarValueModel<T>>(std::move(source));
  }

  template<typename T>
  ProxyScalarValueModel<T>::ProxyScalarValueModel(
    std::shared_ptr<ScalarValueModel<Type>> source)
    : m_source(std::move(source)) {}

  template<typename T>
  std::shared_ptr<ScalarValueModel<typename ProxyScalarValueModel<T>::Type>>
      ProxyScalarValueModel<T>::get_source() const {
    return std::static_pointer_cast<ScalarValueModel<Type>>(
      m_source.get_source());
  }

  template<typename T>
  void ProxyScalarValueModel<T>::set_source(
      std::shared_ptr<ScalarValueModel<Type>> source) {
    m_source.set_source(std::move(source));
  }

  template<typename T>
  boost::optional<typename ProxyScalarValueModel<T>::Scalar>
      ProxyScalarValueModel<T>::get_minimum() const {
    return get_source()->get_minimum();
  }

  template<typename T>
  boost::optional<typename ProxyScalarValueModel<T>::Scalar>
      ProxyScalarValueModel<T>::get_maximum() const {
    return get_source()->get_maximum();
  }

  template<typename T>
  boost::optional<typename ProxyScalarValueModel<T>::Scalar>
      ProxyScalarValueModel<T>::get_increment() const {
    return get_source()->get_increment();
  }

  template<typename T>
  QValidator::State ProxyScalarValueModel<T>::get_state() const {
    return m_source.get_state();
  }

  template<typename T>
  const typename ProxyScalarValueModel<T>::Type&
      ProxyScalarValueModel<T>::get() const {
    return m_source.get();
  }

  template<typename T>
  QValidator::State ProxyScalarValueModel<T>::test(const Type& value) const {
    return m_source.test(value);
  }

  template<typename T>
  QValidator::State ProxyScalarValueModel<T>::set(const Type& value) {
    return m_source.set(value);
  }

  template<typename T>
  boost::signals2::connection ProxyScalarValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_source.connect_update_signal(slot);
  }
}

#endif
