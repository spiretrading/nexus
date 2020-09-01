#ifndef SPIRE_DECIMAL_SPIN_BOX_HPP
#define SPIRE_DECIMAL_SPIN_BOX_HPP
#include <QAbstractSpinBox>
#include "Spire/Ui/RealSpinBox.hpp"

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
      explicit DecimalSpinBox(double value, QWidget* parent = nullptr);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(double minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(double maximum);

      //! Returns the current step value.
      double get_step() const;

      //! Sets the value to increment/decrement by when stepping up or down.
      /*!
        \param step The increment/decrement value.
      */
      void set_step(double step);

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
