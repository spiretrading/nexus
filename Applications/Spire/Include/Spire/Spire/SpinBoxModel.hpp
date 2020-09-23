#ifndef SPIRE_SPIN_BOX_MODEL_HPP
#define SPIRE_SPIN_BOX_MODEL_HPP
#include <QHash>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  template <typename T>
  class SpinBoxModel {
    public:

      SpinBoxModel(T initial, T minimum, T maximum)
        : m_initial(initial),
          m_minimum(minimum),
          m_maximum(maximum) {}

      T get_initial() const {
        return m_initial;
      }

      void set_initial(T initial) {
        m_initial = initial;
      }

      T get_increment(Qt::KeyboardModifiers modifiers) const {
        return m_modifiers[modifiers];
      }

      void set_increment(Qt::KeyboardModifiers modifiers, T increment) {
        m_modifiers[modifiers] = increment;
      }

      T get_minimum() const {
        return m_minimum;
      }

      void set_minimum(T minimum) {
        m_minimum = minimum;
      }

      T get_maximum() const {
        return m_maximum;
      }

      void set_maximum(T maximum) {
        m_maximum = maximum;
      }

    private:
      T m_initial;
      T m_minimum;
      T m_maximum;
      QHash<Qt::KeyboardModifiers, T> m_modifiers;
  };

  using DecimalSpinBoxModel = SpinBoxModel<double>;

  using IntegerSpinBoxModel = SpinBoxModel<std::int64_t>;

  using MoneySpinBoxModel = SpinBoxModel<Nexus::Money>;

  using QuantitySpinBoxModel = SpinBoxModel<Nexus::Quantity>;

  std::shared_ptr<DecimalSpinBoxModel> make_decimal_spin_box_model(
    double initial, double minimum, double maximum);

  std::shared_ptr<IntegerSpinBoxModel> make_integer_spin_box_model(
    std::int64_t initial, std::int64_t minimum, std::int64_t maximum);

  std::shared_ptr<MoneySpinBoxModel> make_money_spin_box_model(
    Nexus::Money initial, Nexus::Money minimum, Nexus::Money maximum);

  std::shared_ptr<QuantitySpinBoxModel> make_quantity_spin_box_model(
    Nexus::Quantity initial, Nexus::Quantity minimum, Nexus::Quantity maximum);
}

#endif
