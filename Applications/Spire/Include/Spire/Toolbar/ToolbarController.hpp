#ifndef SPIRE_TOOLBAR_CONTROLLER_HPP
#define SPIRE_TOOLBAR_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

namespace Spire {
  class BookViewController;
  class UserProfile;

  /** Implements the main application controller for the ToolbarWindow. */
  class ToolbarController {
    public:

      /**
       * Constructs a ToolbarController.
       * @param user_profile The user's profile.
       */
      explicit ToolbarController(Beam::Ref<UserProfile> user_profile);

      ~ToolbarController();

      /** Displays the toolbar window. */
      void open();

      /** Closes the toolbar window and all associated windows. */
      void close();

    private:
      struct EventFilter;
      UserProfile* m_user_profile;
      std::unique_ptr<EventFilter> m_event_filter;
      std::unique_ptr<ToolbarWindow> m_toolbar_window;
      std::shared_ptr<ListModel<BlotterModel*>> m_pinned_blotters;
      std::vector<std::unique_ptr<BookViewController>> m_book_view_controllers;
      std::unique_ptr<KeyBindingsWindow> m_key_bindings_window;
      boost::signals2::scoped_connection m_blotter_added_connection;
      boost::signals2::scoped_connection m_blotter_removed_connection;

      ToolbarController(const ToolbarController&) = delete;
      ToolbarController& operator =(const ToolbarController&) = delete;
      void open_chart_window();
      void open_book_view_window();
      void open_time_and_sales_window();
      void open_canvas_window();
      void open_watchlist_window();
      void open_order_imbalance_indicator_window();
      void open_account_directory_window();
      void open_portfolio_window();
      void open_key_bindings_window();
      void open_profile_window();
      void on_open(ToolbarWindow::WindowType window);
      void on_reopen(const LegacyUI::WindowSettings& settings);
      void on_open_blotter(BlotterModel& model);
      void on_minimize_all();
      void on_restore_all();
      void on_import(
        UserSettings::Categories categories, const std::filesystem::path& path);
      void on_export(
        UserSettings::Categories categories, const std::filesystem::path& path);
      void on_new_blotter(const QString& name);
      void on_blotter_added(BlotterModel& blotter);
      void on_blotter_removed(BlotterModel& blotter);
      void on_book_view_closed(BookViewController& controller);
      void on_key_bindings_window_closed();
      void on_sign_out();
  };
}

#endif
