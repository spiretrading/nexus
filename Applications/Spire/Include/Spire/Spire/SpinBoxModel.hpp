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

      SpinBoxModel(T initial, T minimum, T maximmum,
        QHasH<QKeySequence, T> sequences)
        : m_initial(initial),
          m_minimum(minimum),
          m_maximum(maximum),
          m_sequences(std::move(sequences)) {}

      T get_initial() const {
        return m_initial;
      }

      void set_initial(T initial) {
        m_initial = initial;
      }

      T get_increment(const QKeySequence& sequence) const {
        return m_sequences[sequence];
      }

      void set_increment(const QKeySequence& sequence, T increment) {
        m_sequences[sequence] = increment;
      }

      T get_mininum() const {
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
      QHash<QKeySequence, T> m_sequences;
  };

  using DecimalSpinBoxModel = SpinBoxModel<double>;

  using IntegerSpinBoxModel = SpinBoxModel<std::int64_t>;

  using MoneySpinBoxModel = SpinBoxModel<Nexus::Money>;

  using QuantitySpinBoxModel = SpinBoxModel<Nexus::Quantity>;
}

#endif
