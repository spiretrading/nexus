#ifndef SPIRE_INTEGER_INPUT_WIDGET_HPP
#define SPIRE_INTEGER_INPUT_WIDGET_HPP
#include "Spire/Ui/NumericInputWidget.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying integers.
  class IntegerInputWidget : public QWidget {
    public:

      //! Signals a user interaction with the value.
      using ValueSignal = Signal<void (std::uint64_t value)>;

      //! Constructs an IntegerInputWidget.
      /*!
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit IntegerInputWidget(std::uint64_t value,
        QWidget* parent = nullptr);

      //! Sets the minimum accepted value.
      /*!
        \param minimum The minimum value.
      */
      void set_minimum(std::uint64_t minimum);

      //! Sets the maximum accepted value.
      /*!
        \param maximum The maximum value.
      */
      void set_maximum(std::uint64_t maximum);

      //! Returns the last submitted value.
      std::uint64_t get_value() const;

      //! Sets the current displayed value.
      /*!
        \param value The current value.
      */
      void set_value(std::uint64_t value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ValueSignal::slot_type& slot) const;

      //! Connects a slot to the value commit signal.
      boost::signals2::connection connect_commit_signal(
        const ValueSignal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ValueSignal m_change_signal;
      mutable ValueSignal m_commit_signal;
      NumericInputWidget* m_input_widget;

      void on_editing_finished();
  };
}

#endif
