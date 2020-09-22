#ifndef SPIRE_DECIMAL_SPIN_BOX_HPP
#define SPIRE_DECIMAL_SPIN_BOX_HPP
#include <QAbstractSpinBox>
#include "Spire/Spire/SpinBoxModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying decimal numbers.
  class DecimalSpinBox : public QAbstractSpinBox {
    public:

      //! Signals that the value was modified.
      /*!
        \param value The current value.
      */
      using ChangeSignal = Signal<void (double value)>;

      //! Constructs a DecimalSpinBox.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit DecimalSpinBox(std::shared_ptr<DecimalSpinBoxModel> model,
        QWidget* parent = nullptr);

      //! Returns the last submitted value.
      double get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(double value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    private:
      mutable ChangeSignal m_change_signal;
      RealSpinBox* m_spin_box;
  };
}

#endif
