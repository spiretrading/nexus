#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <QWidget>
#include "spire/toolbar/toolbar.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! \brief Displays the toolbar window.
  class toolbar_window : public QWidget {
    public:

      //! Signals that this window has closed.
      using closed_signal = signal<void ()>;

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

      // Constructs a toolbar_window.
      /*!
        \param parent The parent widget to toolbar_window.
      */
      toolbar_window(QWidget* parent = nullptr);

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable closed_signal m_closed_signal;

      icon_button* m_account_button;
      icon_button* m_key_bindings_button;
      icon_button* m_canvas_button;
      icon_button* m_bookview_button;
      icon_button* m_time_sale_button;
      icon_button* m_chart_button;
      icon_button* m_dashboard_button;
      icon_button* m_order_imbalances_button;
      icon_button* m_blotter_button;
  };
}

#endif
