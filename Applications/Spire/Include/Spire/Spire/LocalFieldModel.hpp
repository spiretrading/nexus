#ifndef SPIRE_LOCAL_FIELD_MODEL_HPP
#define SPIRE_LOCAL_FIELD_MODEL_HPP
#include <memory>
#include "Spire/Spire/FieldModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  template<typename T>
  class LocalFieldModel : public FieldModel<T> {
    public:
      using Type = typename FieldModel<T>::Type;
      using UpdateSignal = typename FieldModel<T>::UpdateSignal;

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

      using FieldModel<T>::get;
    protected:
      void* get(void* member) const override;

    private:
      Type m_value;
  };

  template<typename T>
  QValidator::State LocalFieldModel<T>::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  const typename LocalFieldModel<T>::Type& LocalFieldModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State LocalFieldModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State LocalFieldModel<T>::set(const Type& value) {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection LocalFieldModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return {};
  }

  template<typename T>
  void* LocalFieldModel<T>::get(void* member) const {
    return nullptr;
  }
}

#endif
