#ifndef SPIRE_DEDUPLICATED_VALUE_MODEL_HPP
#define SPIRE_DEDUPLICATED_VALUE_MODEL_HPP
#include <memory>
#include <utility>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel that signals an update only when its source's
   * value changes.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class DeduplicatedValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a DeduplicatedValueModel.
       * @param source The model to view.
       */
      explicit DeduplicatedValueModel(std::shared_ptr<ValueModel<T>> source);

      QValidator::State get_state() const override;
      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<T>> m_source;
      Type m_value;
      boost::signals2::scoped_connection m_connection;

      void on_update(const Type& value);
  };

  template<typename M>
  DeduplicatedValueModel(std::shared_ptr<M>) ->
    DeduplicatedValueModel<typename M::Type>;

  /**
   * Constructs a DeduplicatedValueModel.
   * @param source The model to view.
   */
  template<typename M>
  auto make_deduplicated_value_model(std::shared_ptr<M> source) {
    return std::make_shared<DeduplicatedValueModel<typename M::Type>>(
      std::move(source));
  }

  template<typename T>
  DeduplicatedValueModel<T>::DeduplicatedValueModel(
      std::shared_ptr<ValueModel<T>> source)
      : m_source(std::move(source)),
        m_value(m_source->get()) {
    m_connection = m_source->connect_update_signal(
      std::bind_front(&DeduplicatedValueModel::on_update, this));
  }

  template<typename T>
  QValidator::State DeduplicatedValueModel<T>::get_state() const {
    return m_source->get_state();
  }

  template<typename T>
  const typename DeduplicatedValueModel<T>::Type&
      DeduplicatedValueModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State DeduplicatedValueModel<T>::test(const Type& value) const {
    return m_source->test(value);
  }

  template<typename T>
  QValidator::State DeduplicatedValueModel<T>::set(const Type& value) {
    return m_source->set(value);
  }

  template<typename T>
  boost::signals2::connection
      DeduplicatedValueModel<T>::connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void DeduplicatedValueModel<T>::on_update(const Type& value) {
    if(value == m_value) {
      return;
    }
    m_value = value;
    m_update_signal(m_value);
  }
}

#endif
