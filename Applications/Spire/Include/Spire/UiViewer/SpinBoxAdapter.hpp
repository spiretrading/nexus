#ifndef SPIRE_SPIN_BOX_TEST_WIDGET_ADAPTER_HPP
#define SPIRE_SPIN_BOX_TEST_WIDGET_ADAPTER_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Provides an adapter for testing a spin box and the associated model.
  class SpinBoxAdapter : public QWidget {
    public:

      //! Signal type for spin box value changes.
      /*
        \param value The updated value.
      */
      using ChangeSignal = Signal<void (const QString& value)>;

      //! Constructs a SpinBoxAdapter.
      /*
        \param parent The parent widget.
      */
      SpinBoxAdapter(QWidget* parent = nullptr);

      virtual ~SpinBoxAdapter() = default;

      //! Replaces the current spin box and associated model, iff the provided
      //! values are valid numbers.
      /*!
        \param initial The model's initial value.
        \param minimum The model's minimum value.
        \param minimum The model's maximum value.
        \param increment The model's default increment.
        \return True iff the spin box and model were reset successfully.
      */
      virtual bool reset(const QString& initial, const QString& minimum,
        const QString& maximum, const QString& increment) = 0;

      //! Returns the model's initial value.
      virtual QString get_initial() const = 0;

      //! Sets the model's initial value iff the given value is a valid number.
      /*!
        \param initial The initial value.
        \return True iff the initial value was set successfully.
      */
      virtual bool set_initial(const QString& initial) = 0;

      //! Returns the model's minimum value.
      virtual QString get_minimum() const = 0;

      //! Sets the model's minimum value iff the given value is a valid number.
      /*!
        \param minimum The minimum value.
        \return True iff the minimum was set successfully.
      */
      virtual bool set_minimum(const QString& minimum) = 0;

      //! Returns the model's maximum value.
      virtual QString get_maximum() const = 0;

      //! Sets the model's maximum value iff the given value is a valid number.
      /*!
        \param maximum The maximum value.
        \return True iff the maximum was set successfully.
      */
      virtual bool set_maximum(const QString& maximum) = 0;

      //! Returns the increment for the given key modifiers.
      /*!
        \param modifiers The key modifiers.
      */
      virtual QString get_increment(Qt::KeyboardModifiers modifiers) const = 0;

      //! Sets the increment for the given key modifiers, iff the given
      //! increment is a valid number.
      /*!
        \param modifiers The key modifiers.
        \param increment The increment for the given key modifiers.
        \return True iff the increment was set successfully.
      */
      virtual bool set_increment(Qt::KeyboardModifiers modifiers,
        const QString& increment) = 0;

      //! Connects a slot to the value change signal.
      virtual boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const = 0;
  };
}

#endif
