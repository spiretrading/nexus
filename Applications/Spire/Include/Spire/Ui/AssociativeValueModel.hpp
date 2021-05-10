#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

namespace std {
  template<typename T>
  struct hash<boost::optional<T>> {
    size_t operator ()(const boost::optional<T>& value) const {
      if(!value) {
        return -3333;
      }
      return boost::hash_value(value.get());
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

      using Type = T;

      /** Constructs an AssociativeValueModel. */
      AssociativeValueModel();

      /**
      * Constructs an AssociativeValueModel with a default value. The default
      * value is the value that will become current if all associated models
      * have a value of false. If the provided value is none, there is no
      * default value.
      * @param default_value The default value.
      */
      AssociativeValueModel(const boost::optional<T>& default_value);

      /**
       * Associates a BooleanModel iff it's not already associated.
       * If there are no existing models the BooleanModel is set to true,
       * otherwise, it's set to false.
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
      boost::optional<Type> m_default_value;
      std::unordered_map<Type, std::shared_ptr<BooleanModel>> m_models;
      bool m_is_blocked;

      void set_associated_model_value(const Type& value, bool model_value);
      void on_current(const Type& value, bool is_selected);
  };

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel()
    : AssociativeValueModel(boost::none) {}

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel(
      const boost::optional<T>& default_value)
    : m_is_blocked(false),
      m_default_value(default_value) {}

  template<typename T>
  void AssociativeValueModel<T>::associate(
      const std::shared_ptr<BooleanModel>& model, const T& value) {
    auto inserted_model = m_models.insert({value, model});
    if(!inserted_model.second) {
      return;
    }
    if(model->get_current()) {
      model->set_current(false);
    }
    model->connect_current_signal([=] (auto is_selected) {
      on_current(value, is_selected);
    });
    if(get_state() == QValidator::Invalid) {
      set_current(value);
    }
  }

  template<typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::make_association(
      const T& value) {
    auto model = std::make_shared<LocalBooleanModel>(false);
    associate(model, value);
    return model;
  }

  template<typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::find(
      const T& value) const {
    auto iterator = m_models.find(value);
    if(iterator == m_models.end()) {
      return nullptr;
    }
    return iterator->second;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    if(m_models.find(m_current) != m_models.end()) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template<typename T>
  const typename AssociativeValueModel<T>::Type&
    AssociativeValueModel<T>::get_current() const {
      return m_current;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::set_current(const T& value) {
    if(m_models.find(value) == m_models.end()) {
      return QValidator::Invalid;
    }
    auto previous = m_current;
    m_current = value;
    set_associated_model_value(previous, false);
    set_associated_model_value(m_current, true);
    m_current_signal(m_current);
    return QValidator::Acceptable;
  }

  template<typename T>
  boost::signals2::connection AssociativeValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename T>
  void AssociativeValueModel<T>::set_associated_model_value(const T& value,
      bool model_value) {
    if(auto iterator = m_models.find(value); iterator != m_models.end()) {
      iterator->second->set_current(model_value);
    }
  }

  template<typename T>
  void AssociativeValueModel<T>::on_current(const T& value, bool is_selected) {
    if(value == m_current && is_selected ||
        value != m_current && !is_selected) {
      return;
    }
    if(m_is_blocked) {
      if(is_selected) {
        set_associated_model_value(value, false);
      }
      return;
    }
    if(m_current == value && !is_selected) {
      if(m_default_value) {
        m_is_blocked = true;
        set_current(*m_default_value);
        m_is_blocked = false;
      } else {
        set_associated_model_value(m_current, true);
      }
      return;
    }
    m_is_blocked = true;
    set_current(value);
    m_is_blocked = false;
  }

  template<typename T>
  std::shared_ptr<AssociativeValueModel<boost::optional<T>>>
      make_nullable_associative_model() {
    auto model =
      std::make_shared<AssociativeValueModel<boost::optional<T>>>(
        boost::optional<T>(boost::none));
    model->associate(std::make_shared<LocalBooleanModel>(true), boost::none);
    return model;
  }
}

#endif
