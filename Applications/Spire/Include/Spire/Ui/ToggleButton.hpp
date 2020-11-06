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
        \param icon The icon shown when the button is in an untoggled state.
        \param toggle_icon The icon shown when the button is in a toggled
                            state.
        \param parent The parent QWidget to the ToggleButton.
      */
      ToggleButton(QImage icon, QImage toggle_icon, QWidget* parent = nullptr);

      //! Constructs a ToggleButton with a specific hover icon.
      /*!
        \param icon The icon shown when the button is in an untoggled state.
        \param toggle_icon The icon shown when the button is in a toggled
                            state.
        \param hover_icon The icon shown when the button is hovered or receives
                          keyboard focus.
        \param parent The parent QWidget to the button
      */
      ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
        QWidget* parent = nullptr);

      //! Constructs a ToggleButton with a specific hover icon.
      /*!
        \param icon The icon shown when the button is in an untoggled state.
        \param toggle_icon The icon shown when the button is in a toggled
                            state.
        \param hover_icon The icon shown when the button is hovered or receives
                          keyboard focus.
        \param disable_icon The icon shown when the button is disabled.
        \param parent The parent QWidget to the button
      */
      ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
        QImage disabled_icon, QWidget* parent = nullptr);

      //! Sets the toggled state of the button.
      /*!
        \param toggled The toggled state of the button.
      */
      void set_toggled(bool toggled);

      //! Sets the status of the button, and applies the appropriate icons.
      /*!
        \param enabled Enables the button if true, disables it otherwise.
      */
      void setEnabled(bool enabled);

      //! Sets the status of the button, and applies the appropriate icons.
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
      QImage m_icon;
      QImage m_toggle_icon;
      QImage m_hover_icon;
      QImage m_disabled_icon;

      void set_icons(bool enabled);
      void swap_toggle();
  };
}

#endif
