#ifndef SPIRE_TIME_IN_FORCE_COMBO_BOX_HPP
#define SPIRE_TIME_IN_FORCE_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting TimeInForce values.
  class TimeInForceComboBox : public QLineEdit {
    public:

      //! Constructs a TimeInForceComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit TimeInForceComboBox(QWidget* parent = nullptr);

      //! Returns the current TimeInForce.
      Nexus::TimeInForce get_time_in_force() const;

      //! Sets the current TimeInForce.
      void set_time_in_force(Nexus::TimeInForce time);

    private:
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
