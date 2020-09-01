#ifndef SPIRE_QUANTITY_SPIN_BOX_HPP
#define SPIRE_QUANTITY_SPIN_BOX_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying Quantities.
  class QuantitySpinBox : public QAbstractSpinBox {
    public:

      //! Signals that the value was modified.
      /*!
        \param value The current value.
      */
      using ChangeSignal = Signal<void (Nexus::Quantity value)>;

      //! Constructs a QuantitySpinBox.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit QuantitySpinBox(Nexus::Quantity value,
        QWidget* parent = nullptr);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(Nexus::Quantity minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(Nexus::Quantity maximum);

      //! Returns the current step value.
      Nexus::Quantity get_step() const;

      //! Sets the value to increment/decrement by when stepping up or down.
      /*!
        \param step The increment/decrement value.
      */
      void set_step(Nexus::Quantity step);

      //! Returns the last submitted value.
      Nexus::Quantity get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(Nexus::Quantity value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    private:
      mutable ChangeSignal m_change_signal;
      RealSpinBox* m_spin_box;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      RealSpinBox::Real to_real(Nexus::Quantity value);
  };
}

#endif
