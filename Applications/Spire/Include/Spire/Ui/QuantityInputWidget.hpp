#ifndef SPIRE_QUANTITY_INPUT_WIDGET_HPP
#define SPIRE_QUANTITY_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays an input box that accepts Quantities.
  class QuantityInputWidget : public QLineEdit {
    public:

      //! Signals that the user has modified the input box or submitted a
      //! value.
      using InputSignal = Signal<void (Nexus::Quantity)>;

      //! Constructs a QuantityInputWidget.
      /*
        \param parent The parent widget.
      */
      explicit QuantityInputWidget(QWidget* parent = nullptr);

      //! Sets the value to display in the input box.
      /*
        \param value The value to display.
      */
      void set_value(Nexus::Quantity value);

      //! Connects a signal to the value committed signal.
      boost::signals2::connection connect_committed_signal(
        const InputSignal::slot_type& slot) const;

      //! Connects a signal to the value modified signal.
      boost::signals2::connection connect_modified_signal(
        const InputSignal::slot_type& slot) const;

    private:
      CustomVariantItemDelegate m_item_delegate;
      mutable InputSignal m_committed_signal;
      mutable InputSignal m_modified_signal;

      void on_line_edit_committed();
      void on_line_edit_modified(const QString& text);
  };
}

#endif
