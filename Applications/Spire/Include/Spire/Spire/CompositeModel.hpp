#ifndef SPIRE_COMPOSITE_MODEL_HPP
#define SPIRE_COMPOSITE_MODEL_HPP
#include <unordered_map>
#include <utility>
#include "Spire/Spire/Field.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T, typename R>
  struct model_type {
    using type =
      std::remove_cvref_t<decltype(std::declval<T>().*std::declval<R>())>;
  };
  template<typename T, typename R>
  using model_type_t = typename model_type<T, R>::type;
}

  template<typename T>
  class CompositeModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      template<typename R>
      void define(R accessor,
        std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> model);

      template<typename R>
      std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> access(
        R accessor);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      struct VirtualFieldUpdater {
        virtual ~VirtualFieldUpdated() = default;
        virtual std::shared_ptr<void> get_model() const = 0;
        virtual void update(Type& value) const = 0;
      };
      template<typename R, typename M>
      struct FieldUpdater final : VirtualFieldUpdater {
        std::shared_ptr<ValueModel<R>> m_model;
        M m_accessor;

        std::shared_ptr<void> get_model() const override;
        void update(Type& value) const override;
      };
      mutable UpdateSignal m_update_signal;
      Type m_value;
      std::unordered_map<Field, std::shared_ptr<void>> m_fields;

      template<typename R, typename U>
      void on_update(R accessor, const U& value);
  };

  template<typename T>
  template<typename R>
  void CompositeModel<T>::define(R accessor,
      std::shared_ptr<ValueModel<Details::model_type_t<T, R>>> model) {
    model->connect_update_signal(std::bind_front(
      &CompositeModel::on_update<R, Details::model_type_t<T, R>>, this,
      accessor));
    m_fields.insert(std::pair(std::move(accessor), std::move(model)));
  }

  template<typename T>
  template<typename R>
  std::shared_ptr<ValueModel<Details::model_type_t<T, R>>>
      CompositeModel<T>::access(R accessor) {
    auto field = m_fields.find(accessor);
    if(field == m_fields.end()) {
      return nullptr;
    }
    return std::static_pointer_cast<ValueModel<Details::model_type_t<T, R>>>(
      field->second);
  }

  template<typename T>
  const typename CompositeModel<T>::Type& CompositeModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State CompositeModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State CompositeModel<T>::set(const Type& value) {
    m_value = value;
    for(auto& field : m_fields) {
      field.second field.first
    }
  }

  template<typename T>
  boost::signals2::connection CompositeModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  template<typename R, typename U>
  void CompositeModel<T>::on_update(R accessor, const U& value) {
    m_value.*accessor = value;
    m_update_signal(m_value);
  }

  template<typename T>
  template<typename R, typename M>
  std::shared_ptr<void> CompositeModel<T>::FieldUpdater<R, M>::get_model()
      const {
    return m_model;
  }

  template<typename T>
  template<typename R, typename M>
  void CompositeModel<T>::FieldUpdater<R, M>::update(Type& value) const {
    m_model->set(value.*m_accessor);
  }
}

#endif
