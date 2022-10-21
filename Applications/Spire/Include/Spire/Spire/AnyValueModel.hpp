#ifndef SPIRE_ANY_VALUE_MODEL_HPP
#define SPIRE_ANY_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel by wrapping an existing ValueModel with a type-safe
   * generic interface.
   */
  class AnyValueModel : public ValueModel<AnyRef> {
    public:
      using Type = ValueModel<AnyRef>::Type;

      using UpdateSignal = ValueModel<AnyRef>::UpdateSignal;

      /**
       * Constructs an AnyValueModel over an existing ValueModel.
       * @param source The ValueModel to wrap.
       */
      template<typename T>
      explicit AnyValueModel(std::shared_ptr<T> source) requires
        std::derived_from<T, ValueModel<typename T::Type>>;

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      struct VirtualValueModel {
        virtual ~VirtualValueModel() = default;
        virtual QValidator::State get_state() const = 0;
        virtual const Type& get() const = 0;
        virtual QValidator::State test(const Type& value) const = 0;
        virtual QValidator::State set(const Type& value) = 0;
        virtual boost::signals2::connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const = 0;
      };
      template<typename T>
      struct WrapperValueModel final : VirtualValueModel {
        mutable UpdateSignal m_update_signal;
        std::shared_ptr<ValueModel<T>> m_model;
        AnyRef m_value;
        boost::signals2::scoped_connection m_connection;

        WrapperValueModel(std::shared_ptr<ValueModel<T>> model);
        QValidator::State get_state() const override;
        const Type& get() const override;
        QValidator::State test(const Type& value) const override;
        QValidator::State set(const Type& value) override;
        boost::signals2::connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const override;
        void on_update(const T& value);
      };
      std::unique_ptr<VirtualValueModel> m_model;
  };

  template<typename T>
  AnyValueModel::AnyValueModel(std::shared_ptr<T> source) requires
    std::derived_from<T, ValueModel<typename T::Type>>
    : m_model(std::make_unique<WrapperValueModel<typename T::Type>>(
        std::move(source))) {}

  template<typename T>
  AnyValueModel::WrapperValueModel<T>::WrapperValueModel(
    std::shared_ptr<ValueModel<T>> model)
    : m_model(std::move(model)),
      m_value(m_model->get()),
      m_connection(m_model->connect_update_signal(
        std::bind_front(&WrapperValueModel::on_update, this))) {}

  template<typename T>
  QValidator::State AnyValueModel::WrapperValueModel<T>::get_state() const {
    return m_model->get_state();
  }

  template<typename T>
  const AnyValueModel::Type& AnyValueModel::WrapperValueModel<T>::get() const {
    return m_value;
  }

  template<typename T>
  QValidator::State AnyValueModel::WrapperValueModel<T>::test(
      const Type& value) const {
    if(auto p = any_cast<const T>(&value)) {
      return m_model->test(*p);
    }
    return QValidator::State::Invalid;
  }

  template<typename T>
  QValidator::State AnyValueModel::WrapperValueModel<T>::set(
      const Type& value) {
    if(auto p = any_cast<const T>(&value)) {
      return m_model->set(*p);
    } else if(!value.has_value()) {
      return m_model->set(T());
    }
    return QValidator::State::Invalid;
  }

  template<typename T>
  boost::signals2::connection AnyValueModel::WrapperValueModel<T>::
      connect_update_signal(const UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }

  template<typename T>
  void AnyValueModel::WrapperValueModel<T>::on_update(const T& value) {
    m_value = m_model->get();
    m_update_signal(m_value);
  }
}

#endif
