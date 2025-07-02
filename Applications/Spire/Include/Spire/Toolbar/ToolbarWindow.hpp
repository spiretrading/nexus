#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Toolbar/UserSettings.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the toolbar window. */
  class ToolbarWindow : public Window, public LegacyUI::PersistentWindow {
    public:

      /** Enumarates the types of windows that the toolbar can open. */
      enum class WindowType {

        /** The chart window. */
        CHART,

        /** The book view window. */
        BOOK_VIEW,

        /** The time and sales window. */
        TIME_AND_SALES,

        /** The blotter window. */
        BLOTTER,

        /** The canvas window. */
        CANVAS,

        /** The watchlist window. */
        WATCHLIST,

        /** The order imbalance indicator window. */
        ORDER_IMBALANCE_INDICATOR,

        /** The replay window. */
        REPLAY,

        /** The portfolio window. */
        PORTFOLIO,

        /** The key bindings window. */
        KEY_BINDINGS,

        /** The profile window. */
        PROFILE,

        /** The account directory window. */
        ACCOUNT_DIRECTORY
      };

      /**
       * Signals an operation to open a window.
       * @param type The type of window to open.
       */
      using OpenSignal = Signal<void (WindowType window)>;

      /**
       * Signals to re-open a recently closed window.
       * @param settings The settings for the recently closed window to reopen.
       */
      using ReopenSignal =
        Signal<void (const LegacyUI::WindowSettings& settings)>;

      /**
       * Signals to open a blotter.
       * @param blotter The model of the blotter to open.
       */
      using OpenBlotterSignal = Signal<void (BlotterModel& blotter)>;

      /** Signals that the user is minimizing all windows. */
      using MinimizeAllSignal = Signal<void ()>;

      /** Signals that the user is restoring all windows. */
      using RestoreAllSignal = Signal<void ()>;

      /**
       * Signals to import settings.
       * @param categories The categories to import.
       * @param path The path to the settings file to import from.
       */
      using ImportSignal = Signal<void (UserSettings::Categories categories,
        const std::filesystem::path& path)>;

      /**
       * Signals to export settings.
       * @param categories The categories to export.
       * @param path The path to the settings file to export to.
       */
      using ExportSignal = Signal<void (UserSettings::Categories categories,
        const std::filesystem::path& path)>;

      /**
       * Signals a new blotter was created.
       * @param name The name of the new blotter.
       */
      using NewBlotterSignal = Signal<void (const QString& name)>;

      /** Signals that the user has signed out of the account. */
      using SignOutSignal = Signal<void ()>;

      /**
       * Constructs a ToolbarWindow.
       * @param user_name The user's name.
       * @param is_manager Whether the user is a manager.
       * @param recent_windows The list of the recently closed windows.
       * @param pinned_blotters The list of the user's pinned blotters.
       * @param parent The parent widget.
       */
      ToolbarWindow(Beam::ServiceLocator::DirectoryEntry account,
        Nexus::AdministrationService::AccountRoles roles,
        std::shared_ptr<RecentlyClosedWindowListModel> recently_closed_windows,
        std::shared_ptr<ListModel<BlotterModel*>> pinned_blotters,
        QWidget* parent = nullptr);

      /** Returns the list of recently closed window. */
      const std::shared_ptr<RecentlyClosedWindowListModel>&
        get_recently_closed_windows() const;

      /** Returns the list of pinned blotters. */
      const std::shared_ptr<ListModel<BlotterModel*>>&
        get_pinned_blotters() const;

      /** Connects a slot to the OpenSignal. */
      boost::signals2::connection connect_open_signal(
        const OpenSignal::slot_type& slot) const;

      /** Connects a slot to the ReopenSignal. */
      boost::signals2::connection connect_reopen_signal(
        const ReopenSignal::slot_type& slot) const;

      /** Connects a slot to the OpenBlotterSignal. */
      boost::signals2::connection connect_open_blotter_signal(
        const OpenBlotterSignal::slot_type& slot) const;

      /** Connects a slot to the MinimizeAllSignal. */
      boost::signals2::connection connect_minimize_all_signal(
        const MinimizeAllSignal::slot_type& slot) const;

      /** Connects a slot to the RestoreAllSignal. */
      boost::signals2::connection connect_restore_all_signal(
        const RestoreAllSignal::slot_type& slot) const;

      /** Connects a slot to the ImportSignal. */
      boost::signals2::connection connect_import_signal(
        const ImportSignal::slot_type& slot) const;

      /** Connects a slot to the ExportSignal. */
      boost::signals2::connection connect_export_signal(
        const ExportSignal::slot_type& slot) const;

      /** Connects a slot to the NewBlotterSignal. */
      boost::signals2::connection connect_new_blotter_signal(
        const NewBlotterSignal::slot_type& slot) const;

      /** Connects a slot to the SignOutSignal. */
      boost::signals2::connection connect_sign_out_signal(
        const SignOutSignal::slot_type& slot) const;

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable OpenSignal m_open_signal;
      mutable ReopenSignal m_reopen_signal;
      mutable OpenBlotterSignal m_open_blotter_signal;
      mutable MinimizeAllSignal m_minimize_all_signal;
      mutable RestoreAllSignal m_restore_all_signal;
      mutable ImportSignal m_import_signal;
      mutable ExportSignal m_export_signal;
      mutable SignOutSignal m_sign_out_signal;
      mutable NewBlotterSignal m_new_blotter_signal;
      Beam::ServiceLocator::DirectoryEntry m_account;
      ContextMenu* m_recently_closed_menu;
      ContextMenu* m_blotter_menu;
      LineInputForm* m_new_blotter_form;
      SettingsPanel* m_settings_panel;
      std::shared_ptr<RecentlyClosedWindowListModel> m_recently_closed_windows;
      std::shared_ptr<ListModel<BlotterModel*>> m_pinned_blotters;
      boost::signals2::scoped_connection m_recently_closed_windows_connection;
      boost::signals2::scoped_connection m_pinned_blotter_connection;

      MenuButton* make_window_manager_button();
      MenuButton* make_recently_closed_button() const;
      MenuButton* make_blotter_button();
      void populate_recently_closed_menu();
      void populate_blotter_menu();
      Button* make_icon_tool_button(
        WindowType type, const QString& icon_path, QColor fill,
        QColor hover_color, QColor press_color) const;
      void on_recently_closed_window_operation(
        const RecentlyClosedWindowListModel::Operation& operation);
      void on_import();
      void on_export();
      void on_new_blotter_action();
      void on_new_blotter_submission(const QString& name);
      void on_blotter_operation(
        const ListModel<BlotterModel*>::Operation& operation);
  };

  /** Returns the text representation of a WindowType. */ 
  const QString& to_text(ToolbarWindow::WindowType type);
}

#endif
