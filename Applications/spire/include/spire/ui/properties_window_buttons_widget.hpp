#ifndef PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#define PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the button group for a properties window.
  class properties_window_buttons_widget : public QWidget {
    public:

      //! Signals that a button was clicked.
      using clicked_signal = Signal<void ()>;

      //! Constructs a properties_window_buttons_widget.
      properties_window_buttons_widget(QWidget* parent = nullptr);

      boost::signals2::connection connect_save_as_default_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_load_default_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_reset_default_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_apply_to_all_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_cancel_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_apply_signal(
        const clicked_signal::slot_type& slot) const;

      boost::signals2::connection connect_ok_signal(
        const clicked_signal::slot_type& slot) const;

    private:
      mutable clicked_signal m_save_as_default_signal;
      mutable clicked_signal m_load_default_signal;
      mutable clicked_signal m_reset_default_signal;
      mutable clicked_signal m_apply_to_all_signal;
      mutable clicked_signal m_cancel_signal;
      mutable clicked_signal m_apply_signal;
      mutable clicked_signal m_ok_signal;
  };
}

#endif
