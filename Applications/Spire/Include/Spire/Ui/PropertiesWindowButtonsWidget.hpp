#ifndef PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#define PROPERTIES_WINDOW_BUTTONS_WIDGET_HPP
#include <QWidget>
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays the button group for a properties window.
  class PropertiesWindowButtonsWidget : public QWidget {
    public:

      //! Signals that a button was clicked.
      using ClickedSignal = Signal<void ()>;

      //! Constructs a PropertiesWindowButtonsWidget.
      explicit PropertiesWindowButtonsWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_save_as_default_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_load_default_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_reset_default_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_apply_to_all_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_cancel_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_apply_signal(
        const ClickedSignal::slot_type& slot) const;

      boost::signals2::connection connect_ok_signal(
        const ClickedSignal::slot_type& slot) const;

    private:
      mutable ClickedSignal m_save_as_default_signal;
      mutable ClickedSignal m_load_default_signal;
      mutable ClickedSignal m_reset_default_signal;
      mutable ClickedSignal m_apply_to_all_signal;
      mutable ClickedSignal m_cancel_signal;
      mutable ClickedSignal m_apply_signal;
      mutable ClickedSignal m_ok_signal;
  };
}

#endif
