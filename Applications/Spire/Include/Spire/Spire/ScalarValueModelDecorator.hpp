#ifndef SPIRE_SCALAR_VALUE_MODEL_DECORATOR_HPP
#define SPIRE_SCALAR_VALUE_MODEL_DECORATOR_HPP
#include <functional>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Takes a ValueModel and decorates it with ScalarValueModel's methods.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class ScalarValueModelDecorator : public ScalarValueModel<T> {
    public:
      using Type = ScalarValueModel<T>::Type;
      using Scalar = ScalarValueModel<T>::Scalar;
      using CurrentSignal = ScalarValueModel<T>::CurrentSignal;

      /**
       * Constructs a ScalarValueModelDecorator with the default minimum,
       * maximum, and increment.
       * @param model The model to decorate.
       */
      explicit ScalarValueModelDecorator(
        std::shared_ptr<ValueModel<Type>> model);

      /**
       * Constructs a ScalarValueModelDecorator.
       * @param model The model to decorate.
       * @param minimum The minimum value.
       * @param maximum The maximum value.
       * @param increment The increment.
       */
      ScalarValueModelDecorator(std::shared_ptr<ValueModel<Type>> model,
        boost::optional<Scalar> minimum, boost::optional<Scalar> maximum,
        Scalar increment);

      /** Sets the minimum value. */
      void set_minimum(const boost::optional<Scalar>& minimum);

      /** Sets the maximum value. */
      void set_maximum(const boost::optional<Scalar>& maximum);

      /** Sets the increment. */
      void set_increment(Scalar increment);

      QValidator::State get_state() const override;

      const Type& get() const override;

      QValidator::State set_current(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

      boost::optional<Scalar> get_minimum() const override;

      boost::optional<Scalar> get_maximum() const override;

      Scalar get_increment() const override;

    private:
      std::shared_ptr<ValueModel<Type>> m_model;
      QValidator::State m_state;
      boost::optional<Scalar> m_minimum;
      boost::optional<Scalar> m_maximum;
      Scalar m_increment;
      boost::signals2::scoped_connection m_connection;

      void on_current(const Type& current);
  };

  template<typename T>
  ScalarValueModelDecorator(std::shared_ptr<T>) ->
    ScalarValueModelDecorator<typename T::Type>;

  template<typename T>
  ScalarValueModelDecorator<T>::ScalarValueModelDecorator(
    std::shared_ptr<ValueModel<Type>> model)
    : ScalarValueModelDecorator(std::move(model),
        ScalarValueModel<T>::get_minimum(), ScalarValueModel<T>::get_maximum(),
        ScalarValueModel<T>::get_increment()) {}

  template<typename T>
  ScalarValueModelDecorator<T>::ScalarValueModelDecorator(
    std::shared_ptr<ValueModel<Type>> model, boost::optional<Scalar> minimum,
    boost::optional<Scalar> maximum, Scalar increment)
    : m_model(std::move(model)),
      m_state(m_model->get_state()),
      m_minimum(std::move(minimum)),
      m_maximum(std::move(maximum)),
      m_increment(std::move(increment)),
      m_connection(m_model->connect_current_signal(
        std::bind_front(&ScalarValueModelDecorator::on_current, this))) {}

  template<typename T>
  void ScalarValueModelDecorator<T>::set_minimum(
      const boost::optional<Scalar>& minimum) {
    m_minimum = minimum;
  }

  template<typename T>
  void ScalarValueModelDecorator<T>::set_maximum(
      const boost::optional<Scalar>& maximum) {
    m_maximum = maximum;
  }

  template<typename T>
  void ScalarValueModelDecorator<T>::set_increment(Scalar increment) {
    m_increment = increment;
  }

  template<typename T>
  QValidator::State ScalarValueModelDecorator<T>::get_state() const {
    return m_state;
  }

  template<typename T>
  const typename ScalarValueModelDecorator<T>::Type&
      ScalarValueModelDecorator<T>::get() const {
    return m_model->get();
  }

  template<typename T>
  QValidator::State ScalarValueModelDecorator<T>::set_current(
      const Type& value) {
    using namespace std;
    auto has_value = [&] {
      if constexpr(std::is_same_v<Scalar, Type>) {
        return true;
      } else {
        return value.is_initialized();
      }
    }();
    if(has_value) {
      auto& unwrapped_value = [&] () -> decltype(auto) {
        if constexpr(std::is_same_v<Scalar, Type>) {
          return value;
        } else {
          return *value;
        }
      }();
      if constexpr(std::numeric_limits<Type>::is_integer) {
        if(unwrapped_value != m_model->get() &&
            (unwrapped_value % m_increment) != 0) {
          return QValidator::State::Invalid;
        }
      } else if constexpr(std::numeric_limits<Type>::is_specialized) {
        if(unwrapped_value != m_model->get() &&
            fmod(unwrapped_value, m_increment) != 0) {
          return QValidator::State::Invalid;
        }
      }
    }
    {
      auto blocker = boost::signals2::shared_connection_block(m_connection);
      m_state = m_model->set_current(value);
    }
    if(m_state == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    if(has_value) {
      auto& unwrapped_value = [&] () -> decltype(auto) {
        if constexpr(std::is_same_v<Scalar, Type>) {
          return value;
        } else {
          return *value;
        }
      }();
      if(m_minimum && unwrapped_value < *m_minimum ||
          m_maximum && unwrapped_value > *m_maximum) {
        m_state = QValidator::Intermediate;
      }
    }
    return m_state;
  }

  template<typename T>
  boost::signals2::connection
      ScalarValueModelDecorator<T>::connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const {
    return m_model->connect_current_signal(slot);
  }

  template<typename T>
  boost::optional<typename ScalarValueModelDecorator<T>::Scalar>
      ScalarValueModelDecorator<T>::get_minimum() const {
    return m_minimum;
  }

  template<typename T>
  boost::optional<typename ScalarValueModelDecorator<T>::Scalar>
      ScalarValueModelDecorator<T>::get_maximum() const {
    return m_maximum;
  }

  template<typename T>
  typename ScalarValueModelDecorator<T>::Scalar
      ScalarValueModelDecorator<T>::get_increment() const {
    return m_increment;
  }

  template<typename T>
  void ScalarValueModelDecorator<T>::on_current(const Type& current) {
    m_state = m_model->get_state();
  }
}

#endif
