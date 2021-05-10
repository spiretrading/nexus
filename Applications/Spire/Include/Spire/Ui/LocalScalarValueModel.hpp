#ifndef SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#define SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#include <utility>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ScalarValueModel by storing and updating a local value.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalScalarValueModel : public ScalarValueModel<T> {
    public:
      using Type = typename ScalarValueModel<T>::Type;

      using Scalar = typename ScalarValueModel<T>::Scalar;

      using CurrentSignal = typename ScalarValueModel<T>::CurrentSignal;

      /** Constructs a default model. */
      LocalScalarValueModel();

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalScalarValueModel(Type current);

      /** Sets the minimum value assignable to current. */
      void set_minimum(const boost::optional<Scalar>& minimum);

      /** Sets the maximum value assignable to current. */
      void set_maximum(const boost::optional<Scalar>& maximum);

      /** Sets the increment. */
      void set_increment(const Scalar& increment);

      boost::optional<Scalar> get_minimum() const override;

      boost::optional<Scalar> get_maximum() const override;

      Scalar get_increment() const override;

      QValidator::State get_state() const override;

      const Type& get_current() const override;

      QValidator::State set_current(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      LocalValueModel<Type> m_model;
      QValidator::State m_state;
      boost::optional<Scalar> m_minimum;
      boost::optional<Scalar> m_maximum;
      Scalar m_increment;
  };

  template<typename T>
  LocalScalarValueModel<T>::LocalScalarValueModel()
    : m_state(m_model.get_state()),
      m_increment(std::numeric_limits<Scalar>::max() /
        std::numeric_limits<Scalar>::max()) {}

  template<typename T>
  LocalScalarValueModel<T>::LocalScalarValueModel(Type current)
    : m_model(std::move(current)),
      m_state(m_model.get_state()),
      m_increment(std::numeric_limits<Scalar>::max() /
        std::numeric_limits<Scalar>::max()) {}

  template<typename T>
  void LocalScalarValueModel<T>::set_minimum(
      const boost::optional<Scalar>& minimum) {
    m_minimum = minimum;
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_maximum(
      const boost::optional<Scalar>& maximum) {
    m_maximum = maximum;
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_increment(const Scalar& increment) {
    m_increment = increment;
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Scalar>
      LocalScalarValueModel<T>::get_minimum() const {
    return m_minimum;
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Scalar>
      LocalScalarValueModel<T>::get_maximum() const {
    return m_maximum;
  }

  template<typename T>
  typename LocalScalarValueModel<T>::Scalar
      LocalScalarValueModel<T>::get_increment() const {
    return m_increment;
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::get_state() const {
    return m_state;
  }

  template<typename T>
  const typename LocalScalarValueModel<T>::Type&
      LocalScalarValueModel<T>::get_current() const {
    return m_model.get_current();
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::set_current(const Type& value) {
    using namespace std;
    if(value) {
      if constexpr(std::numeric_limits<Type>::is_integer) {
        if(*value != m_model.get_current() && (*value % m_increment) != 0) {
          return QValidator::State::Invalid;
        }
      } else {
        if(*value != m_model.get_current() && fmod(*value, m_increment) != 0) {
          return QValidator::State::Invalid;
        }
      }
    }
    m_state = m_model.set_current(value);
    if(value) {
      if(m_minimum && *value < *m_minimum ||
          m_maximum && *value > *m_maximum) {
        m_state = QValidator::Intermediate;
      }
    }
    return m_state;
  }

  template<typename T>
  boost::signals2::connection LocalScalarValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_model.connect_current_signal(slot);
  }
}

#endif
