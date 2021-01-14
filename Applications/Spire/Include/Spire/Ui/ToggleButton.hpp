#ifndef SPIRE_TOGGLE_BUTTON_HPP
#define SPIRE_TOGGLE_BUTTON_HPP
#include <QAbstractButton>
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a button using an icon that can be checked.
  class ToggleButton : public QWidget {
    public:

      //! Signals that the ToggleButton was selected.
      using SelectedSignal = Signal<void ()>;

      //! Constructs a ToggleButton.
      /*!
        \param icon The button's icon.
        \param parent The parent QWidget to the ToggleButton.
      */
      explicit ToggleButton(QImage icon, QWidget* parent = nullptr);

      //! Sets the checked state of the button.
      /*!
        \param is_checked True iff the ToggleButton should be checked.
      */
      void set_checked(bool is_checked);

      //! Sets the status of the button, and applies the appropriate style.
      /*!
        \param enabled Enables the button iff enabled is true.
      */
      void setEnabled(bool enabled);

      //! Sets the status of the button, and applies the appropriate style.
      /*!
        \param disabled Disables the button iff disabled is true.
      */
      void setDisabled(bool disabled);

      //! Connects a slot to the selected signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      QSize sizeHint() const override;

    private:
      mutable SelectedSignal m_selected_signal;
      QImage m_icon;
      IconButton* m_icon_button;
      boost::signals2::scoped_connection m_clicked_connection;
      bool m_is_checked;

      void swap_check_state();
      void update_button();
      void update_button(bool enabled);
  };
}

#endif
