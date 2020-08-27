#ifndef SPIRE_MONEY_SPIN_BOX_HPP
#define SPIRE_MONEY_SPIN_BOX_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying Money values.
  class MoneySpinBox : public QAbstractSpinBox {
    public:

      //! Signals that the value was modified.
      /*!
        \param value The current value.
      */
      using ChangeSignal = Signal<void (Nexus::Money value)>;

      //! Constructs a MoneySpinBox.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit MoneySpinBox(Nexus::Money value,
        QWidget* parent = nullptr);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(Nexus::Money minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(Nexus::Money maximum);

      //! Returns the last submitted value.
      Nexus::Money get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(Nexus::Money value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    private:
      mutable ChangeSignal m_change_signal;
      RealSpinBox* m_spin_box;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      RealSpinBox::Real to_real(Nexus::Money value);
  };
}

#endif
