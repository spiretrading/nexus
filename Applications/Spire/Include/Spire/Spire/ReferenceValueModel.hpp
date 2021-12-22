#ifndef SPIRE_REFERENCE_VALUE_MODEL_HPP
#define SPIRE_REFERENCE_VALUE_MODEL_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel through a reference to an existing object.
   * @param <T> The type of object to reference.
   */
  template<typename T>
  class ReferenceValueModel : public ValueModel<T> {
    public:
      using Type = typename ValueModel<T>::Type;

      using UpdateSignal = typename ValueModel<T>::UpdateSignal;

      /**
       * Constructs a ReferenceValueModel.
       * @param object The object to reference.
       */
      explicit ReferenceValueModel(Type& object);

      /**
       * Signals an update, used when an update was made to the referenced
       * object from outside of this model.
       */
      void signal_update();

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      mutable typename UpdateSignal m_update_signal;
      Type* m_object;
  };

  template<typename T>
  ReferenceValueModel<T>::ReferenceValueModel(Type& object)
    : m_object(&object) {}

  template<typename T>
  void ReferenceValueModel<T>::signal_update() {
    m_update_signal(*m_object);
  }

  template<typename T>
  const typename ReferenceValueModel<T>::Type&
      ReferenceValueModel<T>::get() const {
    return *m_object;
  }

  template<typename T>
  QValidator::State ReferenceValueModel<T>::test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  template<typename T>
  QValidator::State ReferenceValueModel<T>::set(const Type& value) {
    *m_object = value;
    m_update_signal(value);
    return QValidator::State::Acceptable;
  }

  template<typename T>
  boost::signals2::connection ReferenceValueModel<T>::connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const {
    return m_update_signal.connect(slot);
  }
}

#endif
