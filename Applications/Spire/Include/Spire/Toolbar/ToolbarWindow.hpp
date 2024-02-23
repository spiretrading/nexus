#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the toolbar window. */
  class ToolbarWindow : public Window {
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

        /** The imbalance indicator window. */
        IMBALANCE_INDICATOR,

        /** The portfolio viewer window. */
        PORTFOLIO_VIEWER,

        /** The key bindings window. */
        KEY_BINDINGS,

        /** The profile window. */
        PROFILE,
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
      using ReopenSignal = Signal<void (
        const std::shared_ptr<LegacyUI::WindowSettings>& settings)>;

      /**
       * Signals to open a blotter.
       * @param blotter The model of the blotter to open.
       */
      using OpenBlotterSignal = Signal<void (BlotterModel& blotter)>;

      /** Signals that the user is minimizing all windows. */
      using MinimizeAllSignal = Signal<void ()>;

      /** Signals that the user is restoring all windows. */
      using RestoreAllSignal = Signal<void ()>;

      /** Signals that the user has signed out of the account. */
      using SignOutSignal = Signal<void ()>;

      /**
       * The type of model used to store the list of recently closed windows.
       */
      using RecentlyClosedListModel =
        ListModel<std::shared_ptr<LegacyUI::WindowSettings>>;

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
        std::shared_ptr<RecentlyClosedListModel> recently_closed_windows,
        std::shared_ptr<ListModel<BlotterModel*>> pinned_blotters,
        QWidget* parent = nullptr);

      /** Returns the list of recently closed window. */
      const std::shared_ptr<RecentlyClosedListModel>&
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

      /** Connects a slot to the SignOutSignal. */
      boost::signals2::connection connect_sign_out_signal(
        const SignOutSignal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable OpenSignal m_open_signal;
      mutable ReopenSignal m_reopen_signal;
      mutable OpenBlotterSignal m_open_blotter_signal;
      mutable MinimizeAllSignal m_minimize_all_signal;
      mutable RestoreAllSignal m_restore_all_signal;
      mutable SignOutSignal m_sign_out_signal;
      std::shared_ptr<RecentlyClosedListModel> m_recently_closed_windows;
      std::shared_ptr<ListModel<BlotterModel*>> m_pinned_blotters;

      MenuButton* make_window_manager_button() const;
      MenuButton* make_recently_closed_button() const;
      MenuButton* make_blotter_button() const;
      Button* make_icon_tool_button(
        WindowType type, const QString& icon_path) const;
  };

  /** Returns the text representation of a WindowType. */ 
  const QString& to_text(ToolbarWindow::WindowType type);
}

#endif
