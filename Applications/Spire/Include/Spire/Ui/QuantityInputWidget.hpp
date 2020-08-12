#ifndef SPIRE_QUANTITY_INPUT_WIDGET_HPP
#define SPIRE_QUANTITY_INPUT_WIDGET_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DecimalInputWidget.hpp"

namespace Spire {

  //! Represents a widget for displaying and modifying Quantities.
  class QuantityInputWidget : public QWidget {
    public:

      //! Signals a user interaction with the value.
      using ValueSignal = Signal<void (Nexus::Quantity value)>;

      //! Constructs a QuantityInputWidget.
      /*
        \param value The initial value to display.
        \param parent The parent widget.
      */
      explicit QuantityInputWidget(Nexus::Quantity value,
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

      //! Sets the current value of the input.
      /*
        \param value The value to display.
      */
      void set_value(Nexus::Quantity value);

      //! Connects a slot to the value change signal.
      boost::signals2::connection connect_change_signal(
        const ValueSignal::slot_type& slot) const;

      //! Connects a slot to the value submit signal.
      boost::signals2::connection connect_submit_signal(
        const ValueSignal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ValueSignal m_change_signal;
      mutable ValueSignal m_submit_signal;
      DecimalInputWidget* m_input_widget;
      CustomVariantItemDelegate m_item_delegate;

      double value_from_text(const QString& text);
      void on_editing_finished();
      void on_value_changed();
  };
}

#endif
