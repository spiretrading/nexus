#ifndef SPIRE_ORDER_STATUS_COMBO_BOX_HPP
#define SPIRE_ORDER_STATUS_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting OrderStatus values.
  class OrderStatusComboBox : public QLineEdit {
    public:

      //! Constructs a OrderStatusComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit OrderStatusComboBox(QWidget* parent = nullptr);

      //! Returns the current OrderStatus.
      Nexus::OrderStatus get_order_status() const;

      //! Sets the current OrderStatus.
      void set_order_status(Nexus::OrderStatus status);

    private:
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
