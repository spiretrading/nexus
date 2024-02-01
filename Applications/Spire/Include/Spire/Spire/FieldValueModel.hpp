#ifndef SPIRE_FIELD_VALUE_MODEL_HPP
#define SPIRE_FIELD_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
namespace Details {
  template<typename T> struct split_pointer_to_member {};

  template<typename T, typename U>
  struct split_pointer_to_member<U T::*> {
    using field = U;
    using object = T;
  };
}

  /**
   * Implements a ValueModel used to access a member variable from within a
   * ValueModel.
   * @param <T> The type of pointer to member variable used to access a field.
   */
  template<typename T>
  class FieldValueModel :
      public ValueModel<typename Details::split_pointer_to_member<T>::field> {
    public:

      /** The type of pointer to member variable used to access the field. */
      using Field = T;

      using Type = typename ValueModel<
        typename Details::split_pointer_to_member<Field>::field>::Type;

      using UpdateSignal = typename ValueModel<
        typename Details::split_pointer_to_member<Field>::field>::UpdateSignal;

      /**
       * Constructs a FieldValueModel.
       * @param model The model whose field is being accessed.
       * @param field A pointer to member variable used to access the field from
       *        the given <i>model</i>.
       */
      FieldValueModel(std::shared_ptr<
        ValueModel<typename Details::split_pointer_to_member<T>::object>> model,
        Field field);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<ValueModel<
        typename Details::split_pointer_to_member<Field>::object>> m_model;
      Field m_field;
      boost::signals2::scoped_connection m_connection;

      void on_update(const typename Details::split_pointer_to_member<
        Field>::object& value);
  };

  /**
   * Constructs a FieldValueModel.
   * @param model The model whose field is being accessed.
   * @param field A pointer to member variable used to access the field from
   *        the given <i>model</i>.
   */
  template<typename T, typename U>
  auto make_field_value_model(std::shared_ptr<T> model, U field) {
    return std::make_shared<FieldValueModel<U>>(
      std::move(model), std::move(field));
  }

  template<typename T>
  FieldValueModel<T>::FieldValueModel(std::shared_ptr<
    ValueModel<typename Details::split_pointer_to_member<T>::object>> model,
    Field field)
    : m_model(std::move(model)),
      m_field(std::move(field)),
      m_connection(m_model->connect_update_signal(
        std::bind_front(&FieldValueModel::on_update, this))) {}

  template<typename T>
  const typename FieldValueModel<T>::Type& FieldValueModel<T>::get() const {
    return m_model->get().*m_field;
  }

  template<typename T>
  QValidator::State FieldValueModel<T>::test(const Type& value) const {
    auto source = m_model->get();
    source.*m_field = value;
    return m_model->test(source);
  }

  template<typename T>
  QValidator::State FieldValueModel<T>::set(const Type& value) {
    auto source = m_model->get();
    source.*m_field = value;
    return m_model->set(source);
  }

  template<typename T>
  boost::signals2::connection FieldValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void FieldValueModel<T>::on_update(const typename
      Details::split_pointer_to_member<Field>::object& value) {
    m_update_signal(value.*m_field);
  }
}

#endif
