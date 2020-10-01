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

      //! Signals that a Side was selected.
      /*!
        \param side The selected Side.
      */
      using SelectedSignal = Signal<void (Nexus::Side side)>;

      //! Constructs a SideComboBox.
      /*!
        \param parent The parent widget.
      */
      explicit SideComboBox(QWidget* parent = nullptr);

      //! Returns the selected Side.
      Nexus::Side get_current_side() const;

      //! Connects a slot to the side selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      boost::signals2::scoped_connection m_value_connection;
      StaticDropDownMenu* m_menu;
  };
}

#endif
