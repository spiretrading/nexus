#ifndef SPIRE_SPIN_BOX_MODEL_HPP
#define SPIRE_SPIN_BOX_MODEL_HPP
#include <QHash>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DecimalSpinBox.hpp"
#include "Spire/Ui/IntegerSpinBox.hpp"
#include "Spire/Ui/MoneySpinBox.hpp"
#include "Spire/Ui/QuantitySpinBox.hpp"

namespace Spire {

  //! Represents the data model for a spin box.
  template<typename T>
  class SpinBoxModel {
    public:

      using Type = T;

      //! Constructs a SpinBoxModel with no modifiers.
      /*!
        \param initial The initial value.
        \param minimum The minimum acceptable value.
        \param maximum The maximum acceptable value.
      */
      SpinBoxModel(Type initial, Type minimum, Type maximum);

      //! Returns the initial value.
      Type get_initial() const;

      //! Sets the initial value.
      /*!
        \param initial The initial value.
      */
      void set_initial(Type initial);

      //! Returns an increment value. Returns the NoModifer value if the
      //! modifiers have no set increment.
      /*
        \param modifiers The modifiers to get the increment value of.
      */
      Type get_increment(Qt::KeyboardModifiers modifiers) const;

      //! Sets an increment.
      /*
        \param modifiers The modifiers to set the increment value of.
        \param increment The increment value.
      */
      void set_increment(Qt::KeyboardModifiers modifiers, Type increment);

      //! Returns the minimum acceptable value.
      Type get_minimum() const;

      //! Sets the minimum acceptable value.
      /*
        \param minimum The minimum value.
      */
      void set_minimum(Type minimum);

      //! Returns the maximum acceptable value.
      Type get_maximum() const;

      //! Sets the maximum acceptable value.
      /*
        \param maximum The maximum value.
      */
      void set_maximum(Type maximum);

    private:
      Type m_initial;
      Type m_minimum;
      Type m_maximum;
      QHash<Qt::KeyboardModifiers, Type> m_modifiers;
  };

  template<typename T>
  SpinBoxModel<T>::SpinBoxModel(T initial, T minimum, T maximum)
    : m_initial(initial),
      m_minimum(minimum),
      m_maximum(maximum) {}

  template<typename T>
  T SpinBoxModel<T>::get_initial() const {
    return m_initial;
  }

  template<typename T>
  void set_initial(T initial) {
    m_initial = initial;
  }

  template<typename T>
  T SpinBoxModel<T>::get_increment(Qt::KeyboardModifiers modifiers) const {
    if(m_modifiers.contains(modifiers)) {
      return m_modifiers[modifiers];
    }
    return m_modifiers[Qt::NoModifier];
  }

  template<typename T>
  void SpinBoxModel<T>::set_increment(Qt::KeyboardModifiers modifiers,
      T increment) {
    m_modifiers[modifiers] = increment;
  }

  template<typename T>
  T SpinBoxModel<T>::get_minimum() const {
    return m_minimum;
  }

  template<typename T>
  void SpinBoxModel<T>::set_minimum(Type minimum) {
    m_minimum = minimum;
  }

  template<typename T>
  T SpinBoxModel<T>::get_maximum() const {
    return m_maximum;
  }

  template<typename T>
  void SpinBoxModel<T>::set_maximum(T maximum) {
    m_maximum = maximum;
  }

  //! Constructs a DecimalSpinBoxModel with a default NoModifier increment
  //! value of 1.
  std::shared_ptr<DecimalSpinBoxModel> make_decimal_spin_box_model(
    double initial, double minimum, double maximum);

  //! Constructs a IntegerSpinBoxModel with a default NoModifier increment
  //! value of 1.
  std::shared_ptr<IntegerSpinBoxModel> make_integer_spin_box_model(
    std::int64_t initial, std::int64_t minimum, std::int64_t maximum);

  //! Constructs a MoneySpinBoxModel with a default NoModifier increment
  //! value of 0.01.
  std::shared_ptr<MoneySpinBoxModel> make_money_spin_box_model(
    Nexus::Money initial, Nexus::Money minimum, Nexus::Money maximum);

  //! Constructs a QuantitySpinBoxModel with a default NoModifier increment
  //! value of 1.
  std::shared_ptr<QuantitySpinBoxModel> make_quantity_spin_box_model(
    Nexus::Quantity initial, Nexus::Quantity minimum, Nexus::Quantity maximum);
}

#endif
