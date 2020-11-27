#ifndef SPIRE_SIDE_COMBO_BOX_HPP
#define SPIRE_SIDE_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Side values.
  class SideComboBox : public QLineEdit {
    public:

      //! Constructs a SideComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit SideComboBox(QWidget* parent = nullptr);

      //! Returns the current Side.
      Nexus::Side get_side() const;

      //! Sets the current Side.
      void set_side(Nexus::Side side);

    private:
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
