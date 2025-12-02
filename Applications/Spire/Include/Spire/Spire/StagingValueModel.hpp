#ifndef SPIRE_STAGING_VALUE_MODEL_HPP
#define SPIRE_STAGING_VALUE_MODEL_HPP
#include <utility>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel that reflects changes made to a target model, but
   * does not propagate changes back to the target until an explicit commit
   * operation is called. This is useful when a model needs to be synchronized
   * with another model, but allow for intermediate changes to be made without
   * propagating those changes further.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class StagingValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a StagingValueModel.
       * @param model The model to synchronize with and commit changes to.
       */
      explicit StagingValueModel(std::shared_ptr<ValueModel<Type>> model);

      /**
       * Commits the value from this model up to the target model. If the
       * target model rejects the value or there is nothing staged, then this
       * is a no-op.
       * @return The state of the target model after the commit.
       */
      QValidator::State commit();

      QValidator::State get_state() const override;
      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable typename UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<Type>> m_model;
      boost::optional<Type> m_value;
      boost::signals2::scoped_connection m_update_connection;

      void on_update(const Type& value);
  };

  template<typename T>
  StagingValueModel(std::shared_ptr<T>) -> StagingValueModel<typename T::Type>;

  template<typename T>
  StagingValueModel<T>::StagingValueModel(
    std::shared_ptr<ValueModel<Type>> model)
    : m_model(std::move(model)),
      m_update_connection(m_model->connect_update_signal(
        std::bind_front(&StagingValueModel::on_update, this))) {}

  template<typename T>
  QValidator::State StagingValueModel<T>::get_state() const {
    if(m_value) {
      return QValidator::State::Intermediate;
    }
    return m_model->get_state();
  }

  template<typename T>
  QValidator::State StagingValueModel<T>::commit() {
    if(!m_value) {
      return m_model->get_state();
    }
    auto state = [&] {
      auto blocker =
        boost::signals2::shared_connection_block(m_update_connection);
      return m_model->set(*m_value);
    }();
    if(state != QValidator::State::Invalid) {
      m_value = boost::none;
    }
    return state;
  }

  template<typename T>
  const typename StagingValueModel<T>::Type&
      StagingValueModel<T>::get() const {
    if(m_value) {
      return *m_value;
    }
    return m_model->get();
  }

  template<typename T>
  QValidator::State StagingValueModel<T>::test(const Type& value) const {
    if(m_model->test(value) == QValidator::State::Acceptable) {
      return QValidator::State::Acceptable;
    }
    return QValidator::State::Intermediate;
  }

  template<typename T>
  QValidator::State StagingValueModel<T>::set(const Type& value) {
    auto state = test(value);
    m_value = value;
    m_update_signal(value);
    return state;
  }

  template<typename T>
  boost::signals2::connection StagingValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void StagingValueModel<T>::on_update(const Type& value) {
    m_value = boost::none;
    m_update_signal(value);
  }
}

#endif
