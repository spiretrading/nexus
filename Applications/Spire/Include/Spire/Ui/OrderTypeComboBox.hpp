#ifndef SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#define SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting OrderType values.
  class OrderTypeComboBox : public QWidget {
    public:

      //! Signals that an OrderType was selected.
      /*!
        \param type The selected OrderType.
      */
      using SelectedSignal = Signal<void (Nexus::OrderType type)>;

      //! Constructs a OrderTypeComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit OrderTypeComboBox(QWidget* parent = nullptr);

      //! Returns the current OrderType.
      Nexus::OrderType get_order_type() const;

      //! Sets the current OrderType.
      void set_order_type(Nexus::OrderType type);

      //! Connects a slot to the OrderType selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
