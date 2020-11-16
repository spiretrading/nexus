#ifndef SPIRE_TOGGLE_BUTTON_HPP
#define SPIRE_TOGGLE_BUTTON_HPP
#include <QAbstractButton>
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon that can be toggled.
  class ToggleButton : public QWidget {
    public:

      //! Signal type for the clicked signal.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a ToggleButton.
      /*!
        \param icon The button's icon.
        \param parent The parent QWidget to the ToggleButton.
      */
      ToggleButton(QImage icon, QWidget* parent = nullptr);

      //! Sets the toggled state of the button.
      /*!
        \param toggled The toggled state of the button.
      */
      void set_toggled(bool toggled);

      //! Sets the status of the button, and applies the appropriate style.
      /*!
        \param enabled Enables the button if true, disables it otherwise.
      */
      void setEnabled(bool enabled);

      //! Sets the status of the button, and applies the appropriate style.
      /*!
        \param disabled Disables the button if true, enables it otherwise.
      */
      void setDisabled(bool disabled);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    private:
      IconButton* m_icon_button;
      boost::signals2::scoped_connection m_clicked_connection;
      bool m_is_toggled;

      void swap_toggle();
      void update_icons();
      void update_icons(bool enabled);
  };
}

#endif
