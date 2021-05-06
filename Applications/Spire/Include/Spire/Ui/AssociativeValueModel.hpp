#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
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

  ///** 
  // * Associates BooleanModels with corresponding values, and ensures that
  // * only a single model has a value of true at one time.
  // */
  //template<typename T>
  //class AssociativeValueModel : public ValueModel<T> {
  //  public:

  //    using Type = T;

  //    /** Constructs an AssociativeValueModel. */
  //    AssociativeValueModel();

  //    /**
  //     * Associates a BooleanModel iff it's not already associated.
  //     * If there are no existing models the BooleanModel is set to true,
  //     * otherwise, it's set to false.
  //     * @param model The model to associate.
  //     * @param value The associated value.
  //     */
  //    void associate(const std::shared_ptr<BooleanModel>& model,
  //      const Type& value);

  //    /**
  //     * Constructs a LocalBooleanModel and associates it with the given
  //     * value.
  //     * @param value The associated value.
  //     * @return The associated model.
  //     */
  //    std::shared_ptr<BooleanModel> make_association(const Type& value);

  //    /**
  //     * Finds the model associated with a value.
  //     * @param value The model's associated value.
  //     * @return The associated model, or nullptr if no associated model was
  //     *         found.
  //     */
  //    std::shared_ptr<BooleanModel> find(const Type& value) const;

  //    /**
  //     * Returns Acceptable if the model's current value is valid, Invalid
  //     * otherwise.
  //     * @return The state of the AssociativeValueModel.
  //     */
  //    QValidator::State get_state() const override;

  //    /**
  //     * Returns the AssociativeValueModel's current value.
  //     * @return The current value.
  //     */
  //    const Type& get_current() const override;

  //    /**
  //     * Sets the current value, iff a model is associated with the value.
  //     * @param value The current value.
  //     * @return Acceptable if the value was set successfully, Invalid
  //     *         otherwise.
  //     */
  //    QValidator::State set_current(const Type& value) override;

  //    /** Connects a slot to the current signal. */
  //    boost::signals2::connection connect_current_signal(
  //      const typename CurrentSignal::slot_type& slot) const override;

  //  private:
  //    mutable CurrentSignal m_current_signal;
  //    Type m_current;
  //    std::unordered_map<Type, Details::AssociatedBooleanModel> m_models;
  //    bool m_is_blocked;

  //    void set_associated_model_value(const Type& value, bool model_value);
  //    void on_current(const Type& value, bool is_selected);
  //};

  /**
   * A specialized AssociativeValueModel, allowing all associated models to have
   * a value of false.
   */
  template<typename T, typename U = boost::optional<T>>
  class AssociativeValueModel : public ValueModel<U> {
    public:

      using Type = T;

      using OptionalType = U;

      AssociativeValueModel() : m_is_blocked(false) {};

      void associate(const std::shared_ptr<BooleanModel>& model,
        const Type& value);

      std::shared_ptr<BooleanModel> make_association(const Type& value);

      std::shared_ptr<BooleanModel> find(const Type& value) const;

      QValidator::State get_state() const;

      /**
       * Returns the current value. The returned value is none if no
       * associated model has a value of true.
       * @return The current value.
       */
      const OptionalType& get_current() const;

      /**
       * Sets the current value if the value has an associated model. If the
       * given value is none, any associated model that has a value of true
       * is set to false.
       * @param value The current value.
       * @return Acceptable if the value was set successfully, Invalid
       *         otherwise.
       */
      QValidator::State set_current(const OptionalType& value);

      boost::signals2::connection connect_current_signal(
        const typename ValueModel<OptionalType>::CurrentSignal::slot_type& slot) const;

    private:
      mutable typename ValueModel<OptionalType>::CurrentSignal
        m_current_signal;
      OptionalType m_current;
      std::unordered_map<Type, Details::AssociatedBooleanModel> m_models;
      bool m_is_blocked;

      void set_associated_model_value(const Type& value, bool model_value);
      void on_current(const Type& value, bool is_selected);
  };

  //template<typename Type, typename OptionalType>
  //AssociativeValueModel<Type, OptionalType>::AssociativeValueModel()
  //  : 

  template<typename Type, typename OptionalType>
  void AssociativeValueModel<Type, OptionalType>::associate(
      const std::shared_ptr<BooleanModel>& model, const Type& value) {
    if(m_models.find(value) != m_models.end()) {
      return;
    }
    m_models.insert_or_assign(value, Details::AssociatedBooleanModel{model,
      model->connect_current_signal([=] (auto is_selected) {
        on_current(value, is_selected);
      })});
    if(model->get_current()) {
      if(m_current) {
        set_associated_model_value(value, false);
      } else {
        set_current(value);
      }
    }
  }

  template<typename Type, typename OptionalType>
  std::shared_ptr<BooleanModel>
      AssociativeValueModel<Type, OptionalType>::make_association(
        const Type& value) {
    auto model = std::make_shared<LocalBooleanModel>(false);
    associate(model, value);
    return model;
  }

  template<typename Type, typename OptionalType>
  std::shared_ptr<BooleanModel>
      AssociativeValueModel<Type, OptionalType>::find(
        const Type& value) const {
    if(auto iterator = m_models.find(value); iterator != m_models.end()) {
      return iterator->second.m_model;
    }
    return nullptr;
  }

  template<typename Type, typename OptionalType>
  QValidator::State
      AssociativeValueModel<Type, OptionalType>::get_state() const {
    if(!m_models.empty()) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template<typename Type, typename OptionalType>
  const OptionalType&
      AssociativeValueModel<Type, OptionalType>::get_current() const {
    return m_current;
  }

  template<typename Type, typename OptionalType>
  QValidator::State AssociativeValueModel<Type, OptionalType>::set_current(
      const OptionalType& value) {
    if(value && m_models.find(*value) == m_models.end()) {
      return QValidator::Invalid;
    }
    if(m_current) {
      set_associated_model_value(*m_current, false);
    }
    m_current = value;
    m_current_signal(m_current);
    return QValidator::Acceptable;
  }

  template<typename Type, typename OptionalType>
  boost::signals2::connection
      AssociativeValueModel<Type, OptionalType>::connect_current_signal(
        const typename ValueModel<OptionalType>::CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename Type, typename OptionalType>
  void AssociativeValueModel<Type, OptionalType>::set_associated_model_value(
      const Type& value, bool model_value) {
    if(auto iterator = m_models.find(value); iterator != m_models.end()) {
      auto blocker = boost::signals2::shared_connection_block(
        iterator->second.m_connection);
      iterator->second.m_model->set_current(model_value);
    }
  }

  template<typename Type, typename OptionalType>
  void AssociativeValueModel<Type, OptionalType>::on_current(const Type& value,
      bool is_selected) {
    if(m_is_blocked && is_selected) {
      set_associated_model_value(value, false);
      return;
    }
    if(is_selected) {
      m_is_blocked = true;
      set_current(value);
      m_is_blocked = false;
    }
  }

  //template<typename T>
  //AssociativeValueModel<T>::AssociativeValueModel()
  //  : m_is_blocked(false) {}

  //template<typename T>
  //void AssociativeValueModel<T>::associate(
  //    const std::shared_ptr<BooleanModel>& model, const T& value) {
  //  if(m_models.find(value) != m_models.end()) {
  //    return;
  //  }
  //  if(model->get_current()) {
  //    model->set_current(false);
  //  }
  //  m_models.insert_or_assign(value, Details::AssociatedBooleanModel{model,
  //    model->connect_current_signal([=] (auto is_selected) {
  //      on_current(value, is_selected);
  //    })});
  //  if(get_state() == QValidator::Invalid) {
  //    set_current(value);
  //  }
  //}

  //template<typename T>
  //std::shared_ptr<BooleanModel> AssociativeValueModel<T>::make_association(
  //    const T& value) {
  //  auto model = std::make_shared<LocalBooleanModel>(false);
  //  associate(model, value);
  //  return model;
  //}

  //template<typename T>
  //std::shared_ptr<BooleanModel> AssociativeValueModel<T>::find(
  //    const T& value) const {
  //  auto iterator = m_models.find(value);
  //  if(iterator == m_models.end()) {
  //    return nullptr;
  //  }
  //  return iterator->second.m_model;
  //}

  //template<typename T>
  //QValidator::State AssociativeValueModel<T>::get_state() const {
  //  if(m_models.find(m_current) != m_models.end()) {
  //    return QValidator::Acceptable;
  //  }
  //  return QValidator::Invalid;
  //}

  //template<typename T>
  //const T& AssociativeValueModel<T>::get_current() const {
  //  return m_current;
  //}

  //template<typename T>
  //QValidator::State AssociativeValueModel<T>::set_current(const T& value) {
  //  if(m_models.find(value) == m_models.end()) {
  //    return QValidator::Invalid;
  //  }
  //  set_associated_model_value(m_current, false);
  //  m_current = value;
  //  set_associated_model_value(m_current, true);
  //  m_current_signal(m_current);
  //  return QValidator::Acceptable;
  //}

  //template<typename T>
  //boost::signals2::connection AssociativeValueModel<T>::connect_current_signal(
  //    const typename CurrentSignal::slot_type& slot) const {
  //  return m_current_signal.connect(slot);
  //}

  //template<typename T>
  //void AssociativeValueModel<T>::set_associated_model_value(const T& value,
  //    bool model_value) {
  //  if(auto iterator = m_models.find(value); iterator != m_models.end()) {
  //    auto blocker = boost::signals2::shared_connection_block(
  //      iterator->second.m_connection);
  //    iterator->second.m_model->set_current(model_value);
  //  }
  //}

  //template<typename T>
  //void AssociativeValueModel<T>::on_current(const T& value, bool is_selected) {
  //  if(value == m_current) {
  //    if(!is_selected) {
  //      set_associated_model_value(m_current, true);
  //    }
  //    return;
  //  } else if(!is_selected) {
  //    return;
  //  }
  //  if(m_is_blocked) {
  //    set_associated_model_value(value, false);
  //    return;
  //  }
  //  set_associated_model_value(m_current, false);
  //  m_is_blocked = true;
  //  set_current(value);
  //  m_is_blocked = false;
  //}
}

#endif
