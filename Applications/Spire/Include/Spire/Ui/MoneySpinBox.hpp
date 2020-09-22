#ifndef SPIRE_MONEY_SPIN_BOX_HPP
#define SPIRE_MONEY_SPIN_BOX_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Spire/SpinBoxModel.hpp"
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
      explicit MoneySpinBox(std::shared_ptr<MoneySpinBoxModel> model,
        QWidget* parent = nullptr);

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
