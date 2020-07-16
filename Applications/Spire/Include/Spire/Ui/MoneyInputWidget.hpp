#ifndef SPIRE_MONEY_INPUT_WIDGET_HPP
#define SPIRE_MONEY_INPUT_WIDGET_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TextInputWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays an input box that accepts Money values.
  class MoneyInputWidget : public TextInputWidget {
    public:

      //! Signals that the user has modified the input box or submitted a
      //! value.
      using InputSignal = Signal<void (Nexus::Money)>;

      //! Constructs a MoneyInputWidget.
      /*
        \param parent The parent widget.
      */
      explicit MoneyInputWidget(QWidget* parent = nullptr);

      //! Sets the value to display in the input box.
      /*
        \param value The value to display.
      */
      void set_value(Nexus::Money value);

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
