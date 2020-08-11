#ifndef SPIRE_INTEGER_INPUT_WIDGET_HPP
#define SPIRE_INTEGER_INPUT_WIDGET_HPP
#include "Spire/Ui/DecimalInputWidget.hpp"

namespace Spire {

  //! Represents a widget for inputting whole numbers.
  class IntegerInputWidget : public DecimalInputWidget {
    public:

      //! Signals a user interaction with the value.
      using ValueSignal = Signal<void (int value)>;

      //! Constructs a ValueInputWidget.
      /*
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit IntegerInputWidget(int value, QWidget* parent = nullptr);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ValueSignal::slot_type& slot) const;

      //! Connects a slot to the value submit signal.
      boost::signals2::connection connect_submit_signal(
        const ValueSignal::slot_type& slot) const;

    private:
      mutable ValueSignal m_change_signal;
      mutable ValueSignal m_submit_signal;

      void on_text_edited(const QString& text);
  };
}

#endif
