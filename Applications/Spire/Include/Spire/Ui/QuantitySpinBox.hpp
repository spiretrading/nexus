#ifndef SPIRE_QUANTITY_SPIN_BOX_HPP
#define SPIRE_QUANTITY_SPIN_BOX_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying Quantities.
  class QuantitySpinBox : public QAbstractSpinBox {
    public:

      //! Signals a user interaction with the value.
      /*!
        \param value The current value.
      */
      using ValueSignal = Signal<void (Nexus::Quantity value)>;

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

      //! Returns the last submitted value.
      Nexus::Quantity get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(Nexus::Quantity value);
      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ValueSignal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ValueSignal m_change_signal;
      mutable ValueSignal m_commit_signal;
      RealSpinBox* m_spin_box;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      std::string display_string(Nexus::Quantity value);
      void on_editing_finished();
  };
}

#endif
