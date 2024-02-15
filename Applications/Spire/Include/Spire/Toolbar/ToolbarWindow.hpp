#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  class ToolbarWindow : public Window {
    public:

      /** The type of window. */
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

      /** The information of the window. */
      struct WindowInfo {

        /** The WindowType. */
        WindowType m_type;

        /** The name of the window. */
        QString m_name;
      };

      /**
       * Signals that the user is opening a resource.
       * @param recent_window 
       */
      using OpenSignal = Signal<void (WindowInfo window_info)>;

      /** Signals that the user is minimizing all windows. */
      using MinimizeAllSignal = Signal<void ()>;

      /** Signals that the user is restoring all windows. */
      using RestoreAllSignal = Signal<void ()>;

      /** Signals that the user has signed out of the account. */
      using SignOutSignal = Signal<void ()>;

      /**
       * Constructs a ToolbarWindow.
       * @param user_name The user's name.
       * @param is_manager Whether the user is a manager.
       * @param recent_windows The list of the last 25 closed windows.
       * @param pinned_blotters The list of the user's pinned blotters
       *        in alphabetical order.
       * @param parent The parent widget.
       */
      ToolbarWindow(QString user_name, bool is_manager,
        std::shared_ptr<ListModel<WindowInfo>> recent_windows,
        std::shared_ptr<ListModel<QString>> pinned_blotters,
        QWidget* parent = nullptr);

      /** Returns the list of recently closed window. */
      const std::shared_ptr<ListModel<WindowInfo>>& get_recent_windows() const;

      /** Returns the list of the user's pinned blotters. */
      const std::shared_ptr<ListModel<QString>>& get_pinned_blotters() const;

      /** Connects a slot to the OpenSignal. */
      boost::signals2::connection connect_open_signal(
        const OpenSignal::slot_type& slot) const;

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
      mutable MinimizeAllSignal m_minimize_all_signal;
      mutable RestoreAllSignal m_restore_all_signal;
      mutable SignOutSignal m_sign_out_signal;
      std::shared_ptr<ListModel<WindowInfo>> m_recent_windows;
      std::shared_ptr<ListModel<QString>> m_pinned_blotters;
      std::vector<Window*> m_windows;
      std::vector<Window*> m_blotters;
      MenuButton* m_recently_closed_windows;

      Button* make_icon_tool_button(WindowType type, const QString& icon_path,
        const QString& open_name = "");
      MenuButton* make_blotter_button();
  };

  /** Returns the text representation of a WindowType. */ 
  const QString& displayText(ToolbarWindow::WindowType type);
}

#endif
