#ifndef SPIRE_TOGGLE_BUTTON_HPP
#define SPIRE_TOGGLE_BUTTON_HPP
#include <QAbstractButton>
#include "spire/ui/icon_button.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a button using an icon that can be toggled.
  class ToggleButton : public QWidget {
    public:

      //! Constructs a ToggleButton.
      /*
        \param icon The icon shown when the button is in an untoggled state.
        \param toggle_icon The icon shown when the button is in a toggled
                            state.
        \param parent The parent QWidget to the ToggleButton.
      */
      ToggleButton(QImage icon, QImage toggle_icon, QWidget* parent = nullptr);

      //! Constructs a ToggleButton with a specific hover icon.
      /*
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
      /*
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

      //! Sets the status of the button, and applies the appropriate icons.
      /*
        \param enabled Enables the button if true, disables it otherwise.
      */
      void setEnabled(bool enabled);

      //! Sets the status of the button, and applies the appropriate icons.
      /*
        \param disabled Disables the button if true, enables it otherwise.
      */
      void setDisabled(bool disabled);

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      IconButton m_icon_button;
      bool m_is_toggled;
      bool m_is_focused;
      QImage m_icon;
      QImage m_toggle_icon;
      QImage m_hover_icon;
      QImage m_disabled_icon;

      void set_icons(bool enabled);
      void swap_toggle();
  };
}

#endif
