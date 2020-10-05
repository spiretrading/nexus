#ifndef SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#define SPIRE_ORDER_TYPE_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting OrderType values.
  class OrderTypeComboBox : public QLineEdit {
    public:

      //! Signals that an OrderType was selected.
      /*!
        \param type The selected OrderType.
      */
      using SelectedSignal = Signal<void (Nexus::OrderType type)>;

      //! Constructs an OrderTypeComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit OrderTypeComboBox(QWidget* parent = nullptr);

      //! Constructs an OrderTypeComboBox.
      /*!
        \param is_cell_input True iff the OrderTypeComboBox is embedded in a
          table.
        \param parent The parent widget.
      */
      explicit OrderTypeComboBox(bool is_cell_widget,
        QWidget* parent = nullptr);

      Qt::Key get_last_key() const { return m_last_key; }

      //! Returns the selected OrderType.
      Nexus::OrderType get_order_type() const;

      void set_order_type(Nexus::OrderType type);

      //! Connects a slot to the OrderType selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      boost::signals2::scoped_connection m_value_connection;
      StaticDropDownMenu* m_menu;
      bool m_is_cell_widget;
      Qt::Key m_last_key;
  };
}

#endif
