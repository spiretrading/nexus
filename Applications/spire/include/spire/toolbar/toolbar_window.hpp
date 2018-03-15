#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <QComboBox>
#include <QWidget>
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! \brief Displays the toolbar window.
  class toolbar_window : public QWidget {
    public:

      //! Signals that this window has closed.
      using closed_signal = signal<void ()>;

      //! Signals that a recently closed window should be re-opened.
      using reopen_signal =
        signal<void (const recently_closed_model::entry& w)>;

      // Constructs a toolbar_window.
      /*!
        \param parent The parent widget to toolbar_window.
      */
      toolbar_window(QWidget* parent = nullptr);

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

      //! Connects a slot to the reopen window signal.
      boost::signals2::connection connect_reopen_signal(
        const reopen_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable closed_signal m_closed_signal;
      mutable reopen_signal m_reopen_signal;

      icon_button* m_minimize_button;
      icon_button* m_close_button;
      QComboBox* m_window_manager_combobox;
      QComboBox* m_recently_closed_combobox;
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
