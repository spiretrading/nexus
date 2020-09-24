#ifndef SPIRE_REAL_SPIN_BOX_MODEL_HPP
#define SPIRE_REAL_SPIN_BOX_MODEL_HPP
#include <QLocale>
#include "Spire/Spire/SpinBoxModel.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a RealSpinBox's data model.
  class RealSpinBoxModel {
    public:

      virtual ~RealSpinBoxModel() {};

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
  class RealSpinBoxModelImpl : public RealSpinBoxModel {
    public:

      using Type = T;

      //! Constructs a RealSpinBoxModelImpl.
      /*
        \param model The source model.
      */
      RealSpinBoxModelImpl(std::shared_ptr<SpinBoxModel<Type>> model);

      RealSpinBox::Real get_increment(
        Qt::KeyboardModifiers modifiers) const override;

      RealSpinBox::Real get_initial() const override;

      RealSpinBox::Real get_minimum() const override;

      RealSpinBox::Real get_maximum() const override;

    private:
      std::shared_ptr<SpinBoxModel<Type>> m_model;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      RealSpinBox::Real variant_to_real(const QVariant& value) const;
      RealSpinBox::Real to_real(std::int64_t value) const;
      RealSpinBox::Real to_real(double value) const;
      RealSpinBox::Real to_real(Nexus::Quantity value) const;
      RealSpinBox::Real to_real(Nexus::Money value) const;
  };

  template<typename T>
  RealSpinBoxModelImpl<T>::RealSpinBoxModelImpl(
      std::shared_ptr<SpinBoxModel<Type>> model)
      : m_model(std::move(model)) {
    m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
      QLocale::OmitGroupSeparator, true));
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::get_increment(
      Qt::KeyboardModifiers modifiers) const {
    return to_real(m_model->get_increment(modifiers));
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::get_initial() const {
    return to_real(m_model->get_initial());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::get_minimum() const {
    return to_real(m_model->get_minimum());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::get_maximum() const {
    return to_real(m_model->get_maximum());
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::variant_to_real(
      const QVariant& value) const {
    return m_item_delegate.displayText(value,
      m_locale).toStdString().c_str();
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::to_real(
      std::int64_t value) const {
    return value;
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::to_real(double value) const {
    return static_cast<long double>(value);
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::to_real(
      Nexus::Quantity value) const {
    return variant_to_real(QVariant::fromValue<Nexus::Quantity>(value));
  }

  template<typename T>
  RealSpinBox::Real RealSpinBoxModelImpl<T>::to_real(
      Nexus::Money value) const {
    return variant_to_real(QVariant::fromValue<Nexus::Money>(value));
  }
}

#endif
