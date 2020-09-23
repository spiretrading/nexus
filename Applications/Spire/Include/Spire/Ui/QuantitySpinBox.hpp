#ifndef SPIRE_QUANTITY_SPIN_BOX_HPP
#define SPIRE_QUANTITY_SPIN_BOX_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/SpinBoxModel.hpp"
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
        \param model The input's model.
        \param parent The parent widget.
      */
      explicit QuantitySpinBox(
        std::shared_ptr<SpinBoxModel<Nexus::Quantity>> model,
        QWidget* parent = nullptr);

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
      std::shared_ptr<SpinBoxModel<Nexus::Quantity>> m_model;
      RealSpinBox* m_spin_box;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      RealSpinBox::Real to_real(Nexus::Quantity value);
  };
}

#endif
