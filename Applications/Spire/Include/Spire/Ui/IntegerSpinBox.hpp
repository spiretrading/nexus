#ifndef SPIRE_INTEGER_SPIN_BOX_HPP
#define SPIRE_INTEGER_SPIN_BOX_HPP
#include <QAbstractSpinBox>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  using IntegerSpinBoxModel = SpinBoxModel<std::int64_t>;

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
        \param model The input's model.
        \param parent The parent widget.
      */
      explicit IntegerSpinBox(std::shared_ptr<IntegerSpinBoxModel> model,
        QWidget* parent = nullptr);

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
      std::shared_ptr<IntegerSpinBoxModel> m_model;
      RealSpinBox* m_spin_box;
  };
}

#endif
