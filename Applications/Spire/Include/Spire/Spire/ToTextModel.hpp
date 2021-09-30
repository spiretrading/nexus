#ifndef SPIRE_TO_TEXT_MODEL_HPP
#define SPIRE_TO_TEXT_MODEL_HPP
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  template<typename T>
  class ToTextModel : public ValueModel<QString> {
    public:

      using Type = T;

      using ToString = std::function<QString (const Type& value)>;

      using ToType =
        std::function<boost::optional<Type> (const QString& value)>;

      ToTextModel(
        std::shared_ptr<ValueModel<Type>> model, const ToType& to_type);

      ToTextModel(std::shared_ptr<ValueModel<Type>> model,
          const ToString& to_string, const ToType& to_type);

      QValidator::State get_state() const override;

      const QString& get_current() const override;

      QValidator::State set_current(const QString& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      mutable CurrentSignal m_current_signal;
      std::shared_ptr<ValueModel<Type>> m_model;
      CustomVariantItemDelegate m_delegate;
      ToString m_to_string;
      ToType m_to_type;
      QString m_current;

      QString to_string(const Type& value);
      void on_current(const Type& current);
  };

  template<typename T>
  ToTextModel<T>::ToTextModel(
    std::shared_ptr<ValueModel<Type>> model, const ToType& to_type)
    : ToTextModel(std::move(model), std::bind(
        &ToTextModel::to_string, this, std::placeholders::_1), to_type) {}

  template<typename T>
  ToTextModel<T>::ToTextModel(std::shared_ptr<ValueModel<Type>> model,
      const ToString& to_string, const ToType& to_type)
      : m_model(std::move(model)),
        m_to_string(to_string),
        m_to_type(to_type),
        m_current(m_to_string(m_model->get_current())) {
    m_model->connect_current_signal([=] (const auto& current) {
      on_current(current);
    });
  }

  template<typename T>
  QValidator::State ToTextModel<T>::get_state() const {
    return m_model->get_state();
  }

  template<typename T>
  const QString& ToTextModel<T>::get_current() const {
    return m_current;
  }

  template<typename T>
  QValidator::State ToTextModel<T>::set_current(const QString& value) {
    if(auto current = m_to_type(value); current) {
      return m_model->set_current(*current);
    }
    return QValidator::Invalid;
  }

  template<typename T>
  boost::signals2::connection ToTextModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename T>
  QString ToTextModel<T>::to_string(const Type& value) {
    return m_delegate.displayText(value);
  }

  template<typename T>
  void ToTextModel<T>::on_current(const Type& current) {
    m_current = m_to_string(current);
    m_current_signal(m_current);
  }
}

#endif
