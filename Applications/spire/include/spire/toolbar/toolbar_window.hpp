#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <vector>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QWidget>
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"
#include "spire/toolbar/toolbar_menu.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the toolbar window.
  class toolbar_window : public QWidget {
    public:

      //! Signals a window should be opened.
      /*!
        \param w The type of window to open.
      */
      using open_signal = Signal<void (recently_closed_model::type w)>;

      //! Signals that this window has closed.
      using ClosedSignal = Signal<void ()>;

      //! Signals that a recently closed window should be re-opened.
      using reopen_signal =
        Signal<void (const recently_closed_model::entry& w)>;

      //! Constructs a toolbar_window.
      /*!
        \param model The model used to populate the recently closed menu.
        \param account The account that logged in.
        \param parent The parent widget to toolbar_window.
      */
      toolbar_window(recently_closed_model& model,
        const Beam::ServiceLocator::DirectoryEntry& account,
        QWidget* parent = nullptr);

      //! Connects a slot to the open signal.
      boost::signals2::connection connect_open_signal(
        const open_signal::slot_type& slot) const;

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

      //! Connects a slot to the reopen window signal.
      boost::signals2::connection connect_reopen_signal(
        const reopen_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable open_signal m_open_signal;
      mutable ClosedSignal m_closed_signal;
      mutable reopen_signal m_reopen_signal;
      recently_closed_model* m_model;
      std::vector<recently_closed_model::entry> m_entries;
      spire::window* m_window;
      QWidget* m_body;
      toolbar_menu* m_window_manager_button;
      toolbar_menu* m_recently_closed_button;
      icon_button* m_account_button;
      icon_button* m_key_bindings_button;
      icon_button* m_canvas_button;
      icon_button* m_book_view_button;
      icon_button* m_time_and_sales_button;
      icon_button* m_chart_button;
      icon_button* m_dashboard_button;
      icon_button* m_order_imbalances_button;
      icon_button* m_blotter_button;

      void entry_added(const recently_closed_model::entry& e);
      void entry_removed(const recently_closed_model::entry& e);
      void on_item_selected(int index);
      void on_open_window(recently_closed_model::type w);
  };
}

#endif
