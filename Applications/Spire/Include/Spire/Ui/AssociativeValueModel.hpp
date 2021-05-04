#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

namespace Spire {

  template <typename T>
  class AssociativeValueModel : public ValueModel<T> {
    public:

      AssociativeValueModel();

      void associate(const std::shared_ptr<BooleanModel>& model,
        const T& value);

      std::shared_ptr<BooleanModel> make_association(const T& value);

      std::shared_ptr<BooleanModel> find(const T& value) const;

      QValidator::State get_state() const;

      const T& get_current() const;

      QValidator::State set_current(const T& value);

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const;

    private:
      struct Model {
        std::shared_ptr<BooleanModel> m_model;
        boost::signals2::scoped_connection m_connection;
      };

      mutable CurrentSignal m_current_signal;
      T m_current;
      std::unordered_map<T, Model> m_models;
      bool m_is_blocked;

      void set_associated_model_value(const T& value, bool model_value);
      void on_current(const T& value, bool is_selected);
  };

  template <typename T>
  AssociativeValueModel<T>::AssociativeValueModel()
    : m_is_blocked(false) {}

  template <typename T>
  void AssociativeValueModel<T>::associate(
      const std::shared_ptr<BooleanModel>& model, const T& value) {
    if(m_models.find(value) != m_models.end()) {
      return;
    }
    if(model->get_current() == true) {
      model->set_current(false);
    }
    m_models[value] = {model, model->connect_current_signal(
      [=] (auto is_selected) { on_current(value, is_selected); })};
    if(get_state() == QValidator::Invalid) {
      set_current(value);
    }
  }

  template <typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::make_association(
      const T& value) {
    auto model = std::make_shared<LocalBooleanModel>(false);
    associate(model, value);
    return model;
  }

  template <typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::find(
      const T& value) const {
    auto iterator = m_models.find(value);
    if(iterator == m_models.end()) {
      return nullptr;
    }
    return iterator->second.m_model;
  }

  template <typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    if(m_models.find(m_current) != m_models.end()) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template <typename T>
  const T& AssociativeValueModel<T>::get_current() const {
    return m_current;
  }

  template <typename T>
  QValidator::State AssociativeValueModel<T>::set_current(const T& value) {
    if(m_models.find(value) != m_models.end()) {
      m_current = value;
      set_associated_model_value(m_current, true);
      m_current_signal(m_current);
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template <typename T>
  boost::signals2::connection AssociativeValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template <typename T>
  void AssociativeValueModel<T>::set_associated_model_value(const T& value,
      bool model_value) {
    if(auto iterator = m_models.find(value); iterator != m_models.end()) {
      auto blocker = boost::signals2::shared_connection_block(
        iterator->second.m_connection);
      iterator->second.m_model->set_current(model_value);
    }
  }

  template <typename T>
  void AssociativeValueModel<T>::on_current(const T& value, bool is_selected) {
    if(value == m_current) {
      if(!is_selected) {
        set_associated_model_value(m_current, true);
      }
      return;
    }
    if(m_is_blocked) {
      set_associated_model_value(value, false);
      return;
    }
    if(value != m_current) {
      set_associated_model_value(m_current, false);
    }
    m_is_blocked = true;
    set_current(value);
    m_is_blocked = false;
  }
}

#endif
