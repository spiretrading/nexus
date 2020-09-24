#ifndef SPIRE_REAL_SPIN_BOX_MODEL_HPP
#define SPIRE_REAL_SPIN_BOX_MODEL_HPP
#include <QLocale>
#include "Spire/Spire/SpinBoxModel.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a RealSpinBox's data model.
  class RealSpinBoxModel {
    public:
      virtual ~RealSpinBoxModel() = default;

      //! Returns an increment value.
      /*
        \param modifiers The modifiers to get the increment value of.
      */
      virtual RealSpinBox::Real get_increment(
        Qt::KeyboardModifiers modifiers) const = 0;

      //! Returns the initial value.
      virtual RealSpinBox::Real get_initial() const = 0;

      //! Returns the minimum acceptable value.
      virtual RealSpinBox::Real get_minimum() const = 0;

      //! Returns the maximum acceptable value.
      virtual RealSpinBox::Real get_maximum() const = 0;
  };

  //! Wraps a SpinBoxModel to provide a generic interface.
  template<typename T>
  class RealSpinBoxAdapterModel : public RealSpinBoxModel {
    public:

      //! The type stored in the model.
      using Type = T;

      //! Constructs a RealSpinBoxModelAdapter.
      /*
        \param model The source model.
      */
      RealSpinBoxAdapterModel(std::shared_ptr<SpinBoxModel<Type>> model);

      RealSpinBox::Real get_increment(
        Qt::KeyboardModifiers modifiers) const override;

      RealSpinBox::Real get_initial() const override;

      RealSpinBox::Real get_minimum() const override;

      RealSpinBox::Real get_maximum() const override;

    private:
      std::shared_ptr<SpinBoxModel<Type>> m_model;
      static const CustomVariantItemDelegate m_item_delegate;
      static const QLocale m_locale;

      static RealSpinBox::Real variant_to_real(const QVariant& value);
      static RealSpinBox::Real to_real(std::int64_t value);
      static RealSpinBox::Real to_real(double value);
      static RealSpinBox::Real to_real(Nexus::Quantity value);
      static RealSpinBox::Real to_real(Nexus::Money value);
  };

  template<typename T>
  RealSpinBoxAdapterModel<T>::RealSpinBoxAdapterModel(
      std::shared_ptr<SpinBoxModel<Type>> model)
      : m_model(std::move(model)) {}

  template<typename T>
  const QLocale RealSpinBoxAdapterModel<T>::m_locale = [] {
    auto locale = QLocale();
    locale.setNumberOptions(m_locale.numberOptions().setFlag(
      QLocale::OmitGroupSeparator, true));
    return locale;
  }();

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::get_increment(
      Qt::KeyboardModifiers modifiers) const {
    return to_real(m_model->get_increment(modifiers));
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::get_initial() const {
    return to_real(m_model->get_initial());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::get_minimum() const {
    return to_real(m_model->get_minimum());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::get_maximum() const {
    return to_real(m_model->get_maximum());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::variant_to_real(
      const QVariant& value) {
    return m_item_delegate.displayText(value, m_locale).toStdString().c_str();
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::to_real(std::int64_t value) {
    return value;
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::to_real(double value) {
    return static_cast<long double>(value);
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::to_real(
      Nexus::Quantity value) {
    return variant_to_real(QVariant::fromValue<Nexus::Quantity>(value));
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxAdapterModel<T>::to_real(Nexus::Money value) {
    return variant_to_real(QVariant::fromValue<Nexus::Money>(value));
  }
}

#endif
