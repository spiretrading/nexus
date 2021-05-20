#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <functional>
#include <queue>
#include <boost/functional/hash.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace std {
  template<typename T>
  struct hash<boost::optional<T>> {
    size_t operator ()(const boost::optional<T>& value) const {
      if(!value) {
        return -3333;
      }
      return boost::hash_value(*value);
    }
  };
}

namespace Spire {

  /** 
   * Associates BooleanModels with corresponding values, and ensures that
   * only a single model has a value of true at one time.
   */
  template<typename T>
  class AssociativeValueModel : public ValueModel<T> {
    public:

      using Type = typename ValueModel<T>::Type;

      /**
       * Constructs an AssociativeValueModel and associates a BooleanModel with
       * a default constructed value as the default value.
       */
      AssociativeValueModel();

      /**
       * Constructs an AssociativeValueModel with a default value. The default
       * value is associated with a BooleanModel that is set to true if all
       * associated models become false.
       * @param default_value The default value.
       */
      explicit AssociativeValueModel(Type default_value);

      /**
       * Constructs an AssociativeValueModel with a default value associated
       * with a given model.
       * @param model The model to associate with the default value.
       * @param default_value The default value.
       */
      AssociativeValueModel(std::shared_ptr<BooleanModel> model,
        Type default_value);

      /**
       * Associates a BooleanModel with the given value. If the given
       * BooleanModel has a value of true it's set to false.
       * @param model The model to associate.
       * @param value The associated value.
       */
      void associate(const std::shared_ptr<BooleanModel>& model,
        const Type& value);

      /**
       * Constructs a LocalBooleanModel and associates it with the given
       * value.
       * @param value The associated value.
       * @return The associated model.
       */
      std::shared_ptr<BooleanModel> make_association(const Type& value);

      /**
       * Finds the model associated with a value.
       * @param value The model's associated value.
       * @return The associated model, or nullptr if no associated model was
       *         found.
       */
      std::shared_ptr<BooleanModel> find(const Type& value) const;

      /**
       * Returns Acceptable if the model's current value is valid, Invalid
       * otherwise.
       * @return The state of the AssociativeValueModel.
       */
      QValidator::State get_state() const override;

      /**
       * Returns the AssociativeValueModel's current value.
       * @return The current value.
       */
      const Type& get_current() const override;

      /**
       * Sets the current value, iff a model is associated with the value.
       * @param value The current value.
       * @return Acceptable if the value was set successfully, Invalid
       *         otherwise.
       */
      QValidator::State set_current(const Type& value) override;

      /** Connects a slot to the current signal. */
      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      mutable CurrentSignal m_current_signal;
      Type m_current;
      Type m_default_value;
      std::unordered_map<Type, std::shared_ptr<BooleanModel>> m_models;
      std::unordered_map<Type, boost::signals2::scoped_connection>
        m_connections;
      std::queue<Type> m_current_queue;
      bool m_is_blocked;

      void set_associated_model_value(const Type& value, bool model_value);
      void on_current(const Type& value, bool is_selected);
  };

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel()
    : AssociativeValueModel(Type{}) {}

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel(Type default_value)
    : AssociativeValueModel(std::make_shared<LocalBooleanModel>(true),
        std::move(default_value)) {}

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel(
      std::shared_ptr<BooleanModel> model, Type default_value)
      : m_current(std::move(default_value)),
        m_default_value(m_current),
        m_is_blocked(false) {
    associate(model, m_default_value);
  }

  template<typename T>
  void AssociativeValueModel<T>::associate(
      const std::shared_ptr<BooleanModel>& model, const Type& value) {
    m_models.insert_or_assign(value, model);
    model->set_current(value == m_current);
    m_connections.insert_or_assign(value, model->connect_current_signal(
      [=] (auto is_selected) { on_current(value, is_selected); }));
  }

  template<typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::make_association(
      const Type& value) {
    auto model = std::make_shared<LocalBooleanModel>(false);
    associate(model, value);
    return model;
  }

  template<typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::find(
      const Type& value) const {
    auto i = m_models.find(value);
    if(i == m_models.end()) {
      return nullptr;
    }
    return i->second;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    return m_models.at(m_current)->get_state();
  }

  template<typename T>
  const typename AssociativeValueModel<T>::Type&
      AssociativeValueModel<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::set_current(const Type& value) {
    if(m_models.find(value) == m_models.end()) {
      return QValidator::Invalid;
    }
    if(m_is_blocked) {
      m_current_queue.push(value);
      return QValidator::Intermediate;
    }
    m_is_blocked = true;
    auto previous = m_current;
    m_current = value;
    m_current_signal(m_current);
    m_is_blocked = false;
    if(!m_current_queue.empty()) {
      auto current_value = m_current_queue.front();
      m_current_queue.pop();
      set_current(current_value);
    }
    if(m_models.at(previous) != m_models.at(m_current)) {
      set_associated_model_value(previous, false);
      set_associated_model_value(m_current, true);
    }
    return QValidator::Acceptable;
  }

  template<typename T>
  boost::signals2::connection AssociativeValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename T>
  void AssociativeValueModel<T>::set_associated_model_value(const Type& value,
      bool model_value) {
    if(auto i = m_models.find(value); i != m_models.end()) {
      i->second->set_current(model_value);
    }
  }

  template<typename T>
  void AssociativeValueModel<T>::on_current(const Type& value,
      bool is_selected) {
    if(value == m_current && is_selected ||
        value != m_current && !is_selected) {
      return;
    } else if(m_current == value && !is_selected) {
      set_associated_model_value(value, true);
      set_current(m_default_value);
      return;
    }
    set_current(value);
  }

  template<typename T>
  std::shared_ptr<AssociativeValueModel<boost::optional<T>>>
      make_nullable_associative_model() {
    auto model =
      std::make_shared<AssociativeValueModel<boost::optional<T>>>(boost::none);
    model->associate(std::make_shared<LocalBooleanModel>(true), boost::none);
    return model;
  }
}

#endif
