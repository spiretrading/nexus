#ifndef SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#define SPIRE_ASSOCIATIVE_VALUE_MODEL_HPP
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {

  template <typename T>
  class AssociativeValueModel : public ValueModel<T> {
    public:

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

      void on_current(const T& value, bool is_selected);
  };

  template <typename T>
  void AssociativeValueModel<T>::associate(
      const std::shared_ptr<BooleanModel>& model, const T& value) {
    m_models.insert(value, {model, model->connect_current_signal(
      [=] (auto is_selected) { on_current(value, is_selected); })});
  }

  template <typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::make_association(
      const T& value) {
    auto model = std::make_shared<BooleanModel>(false);
    associate(model, value);
    return model;
  }

  template <typename T>
  std::shared_ptr<BooleanModel> AssociativeValueModel<T>::find(
      const T& value) const {
    if(m_models.contains(value)) {
      return m_models[value].m_model;
    }
    return nullptr;
  }

  template <typename T>
  QValidator::State AssociativeValueModel<T>::get_state() const {
    if(m_models.contains(m_current)) {
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
    if(m_models.contains(value)) {
      if(m_models.contains(m_current)) {
        m_models[m_current].m_model->set_current(false);
      }
      m_current = value;
      m_models[m_current].m_model->set_current(true);
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
  void AssociativeValueModel<T>::on_current(const T& value, bool is_selected) {
    
  }
}

#endif
