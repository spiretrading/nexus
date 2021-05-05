#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

namespace Spire {

  struct AssociatedBooleanModel {
    std::shared_ptr<BooleanModel> m_model;
    boost::signals2::scoped_connection m_connection;
  };

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
      mutable CurrentSignal m_current_signal;
      T m_current;
      std::unordered_map<T, AssociatedBooleanModel> m_models;
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
      set_associated_model_value(m_current, false);
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
    set_associated_model_value(m_current, false);
    m_is_blocked = true;
    set_current(value);
    m_is_blocked = false;
  }

  template <typename T>
  class AssociativeValueModel<boost::optional<T>> :
      public ValueModel<boost::optional<T>> {
    public:

      AssociativeValueModel();

      void associate(const std::shared_ptr<BooleanModel>& model,
        const boost::optional<T>& value);

      std::shared_ptr<BooleanModel> make_association(
        const boost::optional<T>& value);

      std::shared_ptr<BooleanModel> find(
        const boost::optional<T>& value) const;

      QValidator::State get_state() const;

      const boost::optional<T>& get_current() const;

      QValidator::State set_current(const boost::optional<T>& value);

      boost::signals2::connection connect_current_signal(
        const typename ValueModel<boost::optional<T>>::CurrentSignal::slot_type& slot) const;

    private:
      mutable typename ValueModel<boost::optional<T>>::CurrentSignal
        m_current_signal;
      boost::optional<T> m_current;
      std::unordered_map<T, AssociatedBooleanModel> m_models;
      bool m_is_blocked;

      void set_associated_model_value(const T& value,
        bool model_value);
      void on_current(const boost::optional<T>& value, bool is_selected);
  };

  template <typename T>
  AssociativeValueModel<boost::optional<T>>::AssociativeValueModel()
    : m_is_blocked(false) {}

  template <typename T>
  void AssociativeValueModel<boost::optional<T>>::associate(
      const std::shared_ptr<BooleanModel>& model,
      const boost::optional<T>& value) {
    if(!value) {
      throw std::invalid_argument(
        "Associated BooleanModel must have a corresponding value.");
    }
    if(m_models.find(*value) != m_models.end()) {
      return;
    }

    m_models[*value] = {model, model->connect_current_signal(
      [=] (auto is_selected) { on_current(value, is_selected); })};
    if(model->get_current() == true) {
      if(m_current) {
        set_associated_model_value(*value, false);
      } else {
        set_current(value);
      }
    }
  }

  template <typename T>
  std::shared_ptr<BooleanModel>
      AssociativeValueModel<boost::optional<T>>::make_association(
      const boost::optional<T>& value) {
    auto model = std::make_shared<LocalBooleanModel>(false);
    associate(model, value);
    return model;
  }

  template <typename T>
  std::shared_ptr<BooleanModel>
      AssociativeValueModel<boost::optional<T>>::find(
      const boost::optional<T>& value) const {
    if(!value) {
      return nullptr;
    }
    
    if(auto iterator = m_models.find(*value); iterator != m_models.end()) {
      return iterator->second.m_model;
    }
    return nullptr;
  }

  template <typename T>
  QValidator::State
      AssociativeValueModel<boost::optional<T>>::get_state() const {
    if(!m_models.empty()) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  template <typename T>
  const boost::optional<T>&
      AssociativeValueModel<boost::optional<T>>::get_current() const {
    return m_current;
  }

  template <typename T>
  QValidator::State AssociativeValueModel<boost::optional<T>>::set_current(
      const boost::optional<T>& value) {
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

  template <typename T>
  boost::signals2::connection
      AssociativeValueModel<boost::optional<T>>::connect_current_signal(
      const typename ValueModel<boost::optional<T>>::CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template <typename T>
  void AssociativeValueModel<boost::optional<T>>::set_associated_model_value(
      const T& value, bool model_value) {
    if(auto iterator = m_models.find(value); iterator != m_models.end()) {
      auto blocker = boost::signals2::shared_connection_block(
        iterator->second.m_connection);
      iterator->second.m_model->set_current(model_value);
    }
  }

  template <typename T>
  void AssociativeValueModel<boost::optional<T>>::on_current(
      const boost::optional<T>& value, bool is_selected) {
    if(m_is_blocked && is_selected) {
      set_associated_model_value(*value, false);
      return;
    }
    if(is_selected) {
      m_is_blocked = true;
      set_current(value);
      m_is_blocked = false;
    }
  }
}

#endif
