#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Associates BooleanModels with corresponding values, and ensures that
   * only a single model has a value of true at one time.
   */
  template<typename T>
  class AssociativeValueModel : public ValueModel<T> {
    public:
      using UpdateSignal = typename ValueModel<T>::UpdateSignal;
      using Type = typename ValueModel<T>::Type;

      /**
       * Constructs an AssociativeValueModel using a default constructed value
       * as the model's default value.
       */
      AssociativeValueModel();

      /**
       * Constructs an AssociativeValueModel with a default value used when
       * the BooleanModel whose value is <code>true</code> is set to
       * <code>false</code>.
       * @param default_value The default value.
       */
      explicit AssociativeValueModel(Type default_value);

      /**
       * Returns a BooleanModel whose value is <code>true</code> whenever
       * <code>this</code> model's current value is a specified value, and
       * vice-versa.
       * @param value The value used to engage the returned BooleanModel,
       *        when <code>this</code> has a value of <i>value</i> then the
       *        returned model will have a current value of <code>true</code>
       *        and vice-versa.
       * @return The associated model.
       */
      std::shared_ptr<ValueModel<bool>> get_association(const Type& value);

      /**
       * Finds the model associated with a value if one exists, otherwise
       * returns <code>nullptr</code>.
       * @param value The model's associated value.
       * @return The associated model or <code>nullptr</code> if no associated
       *         model was found.
       */
      std::shared_ptr<ValueModel<bool>> find(const Type& value) const;

      QValidator::State get_state() const override;
      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      struct InnerModel : ValueModel<bool> {
        using Type = ValueModel<bool>::Type;
        mutable UpdateSignal m_update_signal;
        bool m_current;
        std::function<void (bool)> m_slot;

        void update(bool value);
        void signal() const;
        QValidator::State get_state() const override;
        const Type& get() const override;
        QValidator::State test(const Type& value) const override;
        QValidator::State set(const Type& value) override;
        boost::signals2::connection connect_update_signal(
          const typename UpdateSignal::slot_type& slot) const override;
      };
      mutable UpdateSignal m_update_signal;
      std::unordered_map<Type, std::shared_ptr<InnerModel>> m_models;
      const Type* m_default;
      const Type* m_current;

      void on_current(const Type& value, bool current);
  };

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel()
    : AssociativeValueModel(Type{}) {}

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel(Type default_value) {
    auto model = std::make_shared<InnerModel>();
    m_default = &m_models.emplace(std::move(default_value), model).first->first;
    m_current = m_default;
    model->m_current = true;
    model->m_slot = [=, value = m_current] (auto current) {
      on_current(*value, current);
    };
  }

  template<typename T>
  std::shared_ptr<ValueModel<bool>>
      AssociativeValueModel<T>::get_association(const Type& value) {
    auto model = find(value);
    if(!model) {
      auto model = std::make_shared<InnerModel>();
      auto entry = &m_models.emplace(value, model).first->first;
      model->m_current = false;
      model->m_slot = [=] (auto current) { on_current(*entry, current); };
      return model;
    }
    return model;
  }

  template<typename T>
  std::shared_ptr<ValueModel<bool>>
      AssociativeValueModel<T>::find(const Type& value) const {
    auto i = m_models.find(value);
    if(i == m_models.end()) {
      return nullptr;
    }
    return i->second;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  const typename AssociativeValueModel<T>::Type&
      AssociativeValueModel<T>::get() const {
    return *m_current;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::test(const Type& value) const {
    auto i = m_models.find(value);
    if(i == m_models.end()) {
      return QValidator::Invalid;
    }
    return QValidator::Acceptable;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::set(const Type& value) {
    auto i = m_models.find(value);
    if(i == m_models.end()) {
      return QValidator::Invalid;
    }
    auto next_current = i->second;
    auto previous_current = m_models.at(*m_current);
    if(next_current == previous_current) {
      return QValidator::Acceptable;
    }
    previous_current->update(false);
    next_current->update(true);
    m_current = &i->first;
    previous_current->signal();
    next_current->signal();
    auto current = m_current;
    m_update_signal(*current);
    return QValidator::Acceptable;
  }

  template<typename T>
  boost::signals2::connection AssociativeValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void AssociativeValueModel<T>::on_current(const Type& value, bool current) {
    if(current) {
      set(value);
    } else {
      set(*m_default);
    }
  }

  template<typename T>
  void AssociativeValueModel<T>::InnerModel::update(bool value) {
    m_current = value;
  }

  template<typename T>
  void AssociativeValueModel<T>::InnerModel::signal() const {
    auto current = m_current;
    m_update_signal(current);
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::InnerModel::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  const typename AssociativeValueModel<T>::InnerModel::Type&
      AssociativeValueModel<T>::InnerModel::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::InnerModel::test(
      const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::InnerModel::set(
      const Type& value) {
    if(value != m_current) {
      m_slot(value);
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection
      AssociativeValueModel<T>::InnerModel::connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }
}

#endif
