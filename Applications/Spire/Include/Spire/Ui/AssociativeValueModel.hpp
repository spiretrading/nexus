#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <optional>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

namespace Spire {
namespace Details {
  struct AssociatedBooleanModel {
    std::shared_ptr<BooleanModel> m_model;
    boost::signals2::scoped_connection m_connection;
  };
}

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
      * Sets which value should become current if all associated models have
      * a value of false.
      * @param value The default value.
      * @return Acceptable if default value was set successfully, Invalid
      *         otherwise.
      */
      QValidator::State set_default_value(const Type& value);

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
      struct OptionalHash {
        std::size_t operator()(const Type& value) const {
          return 0;
        }
      };

      mutable CurrentSignal m_current_signal;
      Type m_current;
      std::unique_ptr<Type> m_default_value;
      std::unordered_map<Type, Details::AssociatedBooleanModel, OptionalHash>
        m_models;
      bool m_is_blocked;

      void set_associated_model_value(const Type& value, bool model_value);
      void on_current(const Type& value, bool is_selected);
  };

  template<typename T>
  AssociativeValueModel<T>::AssociativeValueModel()
    : m_is_blocked(false) {}

  template<typename T>
  void AssociativeValueModel<T>::associate(
      const std::shared_ptr<BooleanModel>& model, const T& value) {
    if(m_models.find(value) != m_models.end()) {
      return;
    }
    if(model->get_current()) {
      model->set_current(false);
    }
    m_models.insert_or_assign(value, Details::AssociatedBooleanModel{model,
      model->connect_current_signal([=] (auto is_selected) {
        on_current(value, is_selected);
      })});
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
    return iterator->second.m_model;
  }

  template <typename T>
  QValidator::State AssociativeValueModel<T>::set_default_value(
      const Type& value) {
    m_default_value.reset();
    if(m_models.find(value) != m_models.end()) {
      m_default_value = std::make_unique<Type>(value);
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    if(m_models.find(m_current) != m_models.end()) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template<typename T>
  const T& AssociativeValueModel<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  QValidator::State AssociativeValueModel<T>::set_current(const T& value) {
    if(m_models.find(value) == m_models.end()) {
      return QValidator::Invalid;
    }
    set_associated_model_value(m_current, false);
    m_current = value;
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
      auto blocker = boost::signals2::shared_connection_block(
        iterator->second.m_connection);
      iterator->second.m_model->set_current(model_value);
    }
  }

  template<typename T>
  void AssociativeValueModel<T>::on_current(const T& value, bool is_selected) {
    if(value == m_current) {
      if(!is_selected && !m_default_value) {
        set_associated_model_value(m_current, true);
      } else if(m_default_value) {
        m_is_blocked = true;
        set_current(*m_default_value);
        m_is_blocked = false;
      }
      return;
    } else if(!is_selected) {
      return;
    }
    if(m_is_blocked) {
      set_associated_model_value(value, false);
      return;
    }
    set_associated_model_value(m_current, false);
    m_is_blocked = true;
    set_current(value);
    m_is_blocked = false;
  }

  template<typename T>
  std::shared_ptr<AssociativeValueModel<boost::optional<T>>>
      make_nullable_associative_model() {
    auto model = std::make_shared<AssociativeValueModel<boost::optional<T>>>();
    model->associate(std::make_shared<LocalBooleanModel>(true), boost::none);
    model->set_default_value(boost::none);
    return model;
  }
}

#endif
