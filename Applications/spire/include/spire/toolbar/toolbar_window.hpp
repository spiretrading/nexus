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
  class ToolbarWindow : public QWidget {
    public:

      //! Signals a window should be opened.
      /*!
        \param w The type of window to open.
      */
      using OpenSignal = Signal<void (RecentlyClosedModel::Type w)>;

      //! Signals that this window has closed.
      using ClosedSignal = Signal<void ()>;

      //! Signals that a recently closed window should be re-opened.
      using ReopenSignal = Signal<void (const RecentlyClosedModel::Entry& w)>;

      //! Constructs a ToolbarWindow.
      /*!
        \param model The model used to populate the recently closed menu.
        \param account The account that logged in.
        \param parent The parent widget to ToolbarWindow.
      */
      ToolbarWindow(RecentlyClosedModel& model,
        const Beam::ServiceLocator::DirectoryEntry& account,
        QWidget* parent = nullptr);

      //! Connects a slot to the open signal.
      boost::signals2::connection connect_open_signal(
        const OpenSignal::slot_type& slot) const;

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

      //! Connects a slot to the reopen window signal.
      boost::signals2::connection connect_reopen_signal(
        const ReopenSignal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable OpenSignal m_open_signal;
      mutable ClosedSignal m_closed_signal;
      mutable ReopenSignal m_reopen_signal;
      RecentlyClosedModel* m_model;
      std::vector<RecentlyClosedModel::Entry> m_entries;
      spire::window* m_window;
      QWidget* m_body;
      ToolbarMenu* m_window_manager_button;
      ToolbarMenu* m_recently_closed_button;
      icon_button* m_account_button;
      icon_button* m_key_bindings_button;
      icon_button* m_canvas_button;
      icon_button* m_book_view_button;
      icon_button* m_time_and_sales_button;
      icon_button* m_chart_button;
      icon_button* m_dashboard_button;
      icon_button* m_order_imbalances_button;
      icon_button* m_blotter_button;

      void entry_added(const RecentlyClosedModel::Entry& e);
      void entry_removed(const RecentlyClosedModel::Entry& e);
      void on_item_selected(int index);
      void on_open_window(RecentlyClosedModel::Type w);
  };
}

#endif
