#ifndef SPIRE_INTEGER_SPIN_BOX_HPP
#define SPIRE_INTEGER_SPIN_BOX_HPP
#include <QAbstractSpinBox>
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying integers.
  class IntegerSpinBox : public QAbstractSpinBox {
    public:

      //! Signals that the value was modified.
      /*!
        \param value The current value.
      */
      using ChangeSignal = Signal<void (std::int64_t value)>;

      //! Constructs an IntegerSpinBox.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit IntegerSpinBox(std::int64_t value,
        QWidget* parent = nullptr);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(std::int64_t minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(std::int64_t maximum);

      //! Returns the last submitted value.
      std::int64_t get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(std::int64_t value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const;

    private:
      mutable ChangeSignal m_change_signal;
      RealSpinBox* m_spin_box;
  };
}

#endif
