#ifndef SPIRE_DECIMAL_HPP
#define SPIRE_DECIMAL_HPP
#include <memory>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ScalarValueModel.hpp"

namespace Spire {

  /** Represents the floating point type used by the DecimalBox. */
  using Decimal = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<15>>;

  /** Type of model used by the DecimalBox. */
  using DecimalModel = ScalarValueModel<Decimal>;

  /** A ScalarValueModel over optional Decimals. */
  using OptionalDecimalModel = ScalarValueModel<boost::optional<Decimal>>;

  /** A LocalScalarValueModel over Decimals. */
  using LocalDecimalModel = LocalScalarValueModel<Decimal>;

  /** A LocalScalarValueModel over optional Decimals. */
  using LocalOptionalDecimalModel =
    LocalScalarValueModel<boost::optional<Decimal>>;

  /**
   * Base template for converting a value to a Decimal.
   * @param value The value to convert to Decimal.
   * @return The Decimal representation of the <i>value</i>.
   */
  template<typename T>
  Decimal to_decimal(const T& value);

  /**
   * Base template for converting an optional value to an optional Decimal.
   * @param value The optional value to convert to Decimal.
   * @return The optional Decimal representation of the <i>value</i>.
   */
  template<typename T>
  boost::optional<Decimal> to_decimal(const boost::optional<T>& value) {
    if(value) {
      return to_decimal(*value);
    }
    return boost::none;
  }

  /**
   * Base template for converting a Decimal to another scalar type.
   * @param value The Decimal to convert to a scalar.
   * @return The scalar representation of the Decimal <i>value</i>.
   */
  template<typename T>
  T from_decimal(const Decimal& value);

  /**
   * Base template for converting an optional Decimal to an optional scalar.
   * @param value The optional Decimal to convert to an optional scalar.
   * @return The optional scalar representation of the Decimal <i>value</i>.
   */
  template<typename T>
  boost::optional<T> from_decimal(const boost::optional<Decimal>& value) {
    if(value) {
      return from_decimal<T>(*value);
    }
    return boost::none;
  }

  /**
   * Adaptor class for converting an OptionalScalarValueModel to an
   * OptionalDecimalModel.
   * @param <T> The type being modelled to convert.
   */
  template<typename T>
  class ToDecimalModel : public OptionalDecimalModel {
    public:

      /** The type being modelled to convert. */
      using Type = T;

      /** The type of model being converted. */
      using Model = ScalarValueModel<boost::optional<T>>;

      /**
       * Constructs a ToDecimalModel that converts a given model to an
       * OptionalDecimalModel.
       * @param model The model to convert.
       */
      explicit ToDecimalModel(std::shared_ptr<Model> model);

      boost::optional<Decimal> get_minimum() const override;

      boost::optional<Decimal> get_maximum() const override;

      Decimal get_increment() const override;

      QValidator::State get_state() const override;

      const boost::optional<Decimal>& get() const override;

      QValidator::State
        set(const boost::optional<Decimal>& value) override;

      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const override;

    private:
      mutable CurrentSignal m_current_signal;
      std::shared_ptr<Model> m_model;
      boost::optional<Decimal> m_current;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const boost::optional<Type>& current);
  };

  template<typename T>
  ToDecimalModel<T>::ToDecimalModel(std::shared_ptr<Model> model)
    : m_model(std::move(model)),
      m_current(to_decimal(m_model->get())),
      m_current_connection(m_model->connect_current_signal(
        [=] (const auto& current) { on_current(current); })) {}

  template<typename T>
  boost::optional<Decimal> ToDecimalModel<T>::get_minimum() const {
    return to_decimal(m_model->get_minimum());
  }

  template<typename T>
  boost::optional<Decimal> ToDecimalModel<T>::get_maximum() const {
    return to_decimal(m_model->get_maximum());
  }

  template<typename T>
  Decimal ToDecimalModel<T>::get_increment() const {
    return Decimal(1);
  }

  template<typename T>
  QValidator::State ToDecimalModel<T>::get_state() const {
    return m_model->get_state();
  }

  template<typename T>
  const boost::optional<Decimal>& ToDecimalModel<T>::get() const {
    return m_current;
  }

  template<typename T>
  QValidator::State ToDecimalModel<T>::set(
      const boost::optional<Decimal>& value) {
    auto state =
      m_model->set(from_decimal<Type>(value));
    if(state == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    m_current = value;
    m_current_signal(value);
    return state;
  }

  template<typename T>
  boost::signals2::connection ToDecimalModel<T>::connect_current_signal(
      const CurrentSignal::slot_type& slot) const {
    return m_current_signal.connect(slot);
  }

  template<typename T>
  void ToDecimalModel<T>::on_current(const boost::optional<Type>& current) {
    auto value = to_decimal(current);
    m_current = value;
    m_current_signal(value);
  }
}

#endif
