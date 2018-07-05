#ifndef PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#define PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the button group for a properties window.
  class properties_window_buttons_widget : public QWidget {
    public:

      //! Signals that a button was clicked.
      using clicked_signal = signal<void ()>;

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
  };
}

#endif
