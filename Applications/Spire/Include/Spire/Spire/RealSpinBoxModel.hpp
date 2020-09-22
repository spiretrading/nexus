#ifndef SPIRE_REAL_SPIN_BOX_MODEL_HPP
#define SPIRE_REAL_SPIN_BOX_MODEL_HPP
#include <QKeySequence>
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  using Real = RealSpinBox::Real;

  class RealSpinBoxModel {
    public:

      virtual Real get_increment(const QKeySequence& sequence) const = 0;

      virtual Real get_initial() const = 0;

      virtual Real get_minimum() const = 0;

      virtual Real get_maximum() const = 0;
  };

  template <typename T>
  class RealSpinBoxModelImpl : public RealSpinBoxModel {
    public:

      RealSpinBoxModelImpl(std::shared_ptr<SpinBoxModel<T>> model)
        : m_model(std::move(model)) {}

      Real get_increment(const QKeySequence& sequence) const override {
        return to_real(m_model->get_increment(sequence));
      }

      RealSpinBox::Real get_initial() const override {
        return to_real(m_model->get_initial());
      }

      RealSpinBox::Real get_minimum() const override {
        return to_real(m_model->get_minimum());
      }

      RealSpinBox::Real get_maximum() const override {
        return to_real(m_model->get_maximum());
      }

    private:
      std::shared_ptr<SpinBoxModel<T>> m_model;
      CustomVariantItemDelegate m_item_delegate;

      Real variant_to_real(auto value) {
        return m_item_delegate.displayText(
          QVariant::fromValue(value), m_locale).toStdString().c_str();
      }

      Real to_real(int value) {
        return value.extract_signal_long_long();
      }

      Real to_real(double value) {
        return value.extract_double();
      }

      Real to_real(Nexus::Quantity value) {
        return variant_to_real(value);
      }

      Real to_real(Nexus::Money value) {
        return variant_to_real(value);
      }
  };
}

#endif
