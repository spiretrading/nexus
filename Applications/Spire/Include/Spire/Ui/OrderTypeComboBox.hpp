#ifndef SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#define SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting OrderType values.
  class OrderTypeComboBox : public StaticDropDownMenu {
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

      explicit OrderTypeComboBox(bool has_cell_style,
        QWidget* parent = nullptr);

      Nexus::OrderType get_current_order_type() const;

      Nexus::OrderType get_last_order_type() const;

      void set_order_type(Nexus::OrderType type);

      //! Connects a slot to the OrderType selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
