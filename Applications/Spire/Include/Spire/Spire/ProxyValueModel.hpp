#ifndef SPIRE_PROXY_VALUE_MODEL_HPP
#define SPIRE_PROXY_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel that forwards calls to an underlying ValueModel.
   * @param <T> The type being proxied.
   */
  template<typename T>
  class ProxyValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a ProxyValueModel.
       * @param source The model to forward calls to.
       */
      explicit ProxyValueModel(std::shared_ptr<ValueModel<Type>> source);

      /** Returns the model being proxied. */
      const std::shared_ptr<ValueModel<Type>>& get_source() const;

      /** Sets the model to forward calls to. */
      void set_source(std::shared_ptr<ValueModel<Type>> source);

      QValidator::State get_state() const override;

      const Type& get() const;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<Type>> m_source;
      boost::signals2::scoped_connection m_connection;

      void on_update(const Type& value);
  };

  /**
   * Constructs a ProxyValueModel.
   * @param source The model to forward calls to.
   */
  template<typename T>
  auto make_proxy_value_model(std::shared_ptr<T> source) {
    return std::make_shared<ProxyValueModel<typename T::Type>>(
      std::move(source));
  }

  template<typename T>
  ProxyValueModel<T>::ProxyValueModel(
      std::shared_ptr<ValueModel<Type>> source) {
    set_source(std::move(source));
  }

  template<typename T>
  const std::shared_ptr<ValueModel<typename ProxyValueModel<T>::Type>>&
      ProxyValueModel<T>::get_source() const {
    return m_source;
  }

  template<typename T>
  void ProxyValueModel<T>::set_source(
      std::shared_ptr<ValueModel<Type>> source) {
    m_source = std::move(source);
    m_connection = m_source->connect_update_signal(
      std::bind_front(&ProxyValueModel::on_update, this));
    auto value = m_source->get();
    m_update_signal(value);
  }

  template<typename T>
  QValidator::State ProxyValueModel<T>::get_state() const {
    return m_source->get_state();
  }

  template<typename T>
  const typename ProxyValueModel<T>::Type& ProxyValueModel<T>::get() const {
    return m_source->get();
  }

  template<typename T>
  QValidator::State ProxyValueModel<T>::test(const Type& value) const {
    return m_source->test(value);
  }

  template<typename T>
  QValidator::State ProxyValueModel<T>::set(const Type& value) {
    return m_source->set(value);
  }

  template<typename T>
  boost::signals2::connection ProxyValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void ProxyValueModel<T>::on_update(const Type& value) {
    m_update_signal(value);
  }
}

#endif
