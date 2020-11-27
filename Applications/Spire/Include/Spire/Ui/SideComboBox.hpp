#ifndef SPIRE_SIDE_COMBO_BOX_HPP
#define SPIRE_SIDE_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Side values.
  class SideComboBox : public QWidget {
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

      //! Returns the current Side.
      Nexus::Side get_side() const;

      //! Sets the current Side.
      void set_side(Nexus::Side side);

      //! Connects a slot to the side selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
