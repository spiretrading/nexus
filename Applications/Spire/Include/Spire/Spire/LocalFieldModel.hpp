#ifndef SPIRE_LOCAL_FIELD_MODEL_HPP
#define SPIRE_LOCAL_FIELD_MODEL_HPP
#include <memory>
#include <unordered_map>
#include "Spire/Spire/FieldModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  template<typename T>
  struct AccessorModel : public ValueModel<T> {
    using Type = typename ValueModel<T>::Type;
    using UpdateSignal = typename ValueModel<T>::UpdateSignal;

    AccessorModel(cessor)
      : m_instance(&instance),
        m_accessor(accessor) {}

    const Type& get() const {
      return m_accessor.access(*m_instance);
    }

    QValidator::State test(const Type& value) const {
      return QValidator::State::Acceptable;
    }

    QValidator::State set(const Type& value) {
      auto& field = m_accessor.access(*m_instance); = value;
      m_update_signal(value);
      return QValidator::State::Acceptable;
    }

    boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
      return m_update_signal.connect(slot);
    }

    mutable typename UpdateSignal m_update_signal;
    Instance* m_instance;
    FieldPointer m_accessor;
  };

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
      void* get(const FieldPointer& member) const override;

    private:
      mutable typename UpdateSignal m_update_signal;
      Type m_value;
      std::unordered_map<FieldPointer, std::shared_ptr<void>> m_fields;
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
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void* LocalFieldModel<T>::get(const FieldPointer& member) const {
    auto i = m_fields.find(member);
    if(i != m_fields.end()) {
      return &i->second;
    }
  }
}

#endif
