#ifndef SPIRE_TIME_IN_FORCE_COMBO_BOX_HPP
#define SPIRE_TIME_IN_FORCE_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting TimeInForce values.
  class TimeInForceComboBox : public QWidget {
    public:

      //! Signals that a TimeInForce was selected.
      /*!
        \param time The selected TimeInForce.
      */
      using SelectedSignal = Signal<void (Nexus::TimeInForce time)>;

      //! Constructs a TimeInForceComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit TimeInForceComboBox(QWidget* parent = nullptr);

      //! Returns the current TimeInForce.
      Nexus::TimeInForce get_time_in_force() const;

      //! Sets the current TimeInForce.
      void set_time_in_force(Nexus::TimeInForce time);

      //! Connects a slot to the TimeInForce selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
