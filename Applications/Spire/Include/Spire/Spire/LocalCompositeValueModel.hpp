#ifndef SPIRE_LOCAL_COMPOSITE_VALUE_MODEL_HPP
#define SPIRE_LOCAL_COMPOSITE_VALUE_MODEL_HPP
#include <memory>
#include <tuple>
#include <vector>
#include "Spire/Spire/CompositeValueModel.hpp"
#include "Spire/Spire/ReferenceValueModelBox.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a CompositeValueModel by keeping the value as a local member
   * variable.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalCompositeValueModel : public CompositeValueModel<T> {
    public:
      using Type = typename CompositeValueModel<T>::Type;

      using UpdateSignal = typename CompositeValueModel<T>::UpdateSignal;

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

      using CompositeValueModel<T>::get;

    protected:
      const void* get(const FieldPointer& member) const override;

    private:
      struct Entry {
        FieldPointer m_member;
        ReferenceValueModelBox m_model;
      };
      mutable typename UpdateSignal m_update_signal;
      Type m_value;
      mutable std::vector<Entry> m_fields;

      void on_update();
  };

  template<typename T>
  QValidator::State LocalCompositeValueModel<T>::get_state() const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  const typename LocalCompositeValueModel<T>::Type&
      LocalCompositeValueModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State LocalCompositeValueModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State LocalCompositeValueModel<T>::set(const Type& value) {
    m_value = value;
    m_update_signal(value);
    for(auto& field : m_fields) {
      field.m_model.signal_update();
    }
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection
      LocalCompositeValueModel<T>::connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  const void* LocalCompositeValueModel<T>::get(const FieldPointer& member)
      const {
    auto i = std::find_if(m_fields.begin(), m_fields.end(), [&] (auto& field) {
      return field.m_member == member;
    });
    if(i == m_fields.end()) {
      auto model =
        member.make_reference_value_model_box(const_cast<Type&>(m_value));
      model.connect_update_signal(std::bind_front(
        &LocalCompositeValueModel::on_update,
        const_cast<LocalCompositeValueModel*>(this)));
      m_fields.push_back({member, model});
      i = m_fields.end() - 1;
    }
    return &i->m_model.get_model();
  }

  template<typename T>
  void LocalCompositeValueModel<T>::on_update() {
    m_update_signal(m_value);
  }
}

#endif
