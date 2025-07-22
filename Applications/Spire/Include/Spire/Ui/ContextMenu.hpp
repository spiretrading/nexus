#ifndef SPIRE_CONTEXT_MENU_HPP
#define SPIRE_CONTEXT_MENU_HPP
#include <unordered_map>
#include <variant>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/MouseMoveObserver.hpp"
#include "Spire/Ui/PressObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a pop-up context menu. */
  class ContextMenu : public QWidget {
    public:

      /** The type of function called to execute a menu item's action. */
      using Action = std::function<void ()>;

      /** Enumerates the types of menu items. */
      enum class MenuItemType {

        /** The menu item triggers an action. */
        ACTION,

        /** A disabled action. */
        DISABLED_ACTION,

        /** The menu item is a check box. */
        CHECK,

        /** The menu item separates two sections. */
        SEPARATOR,

        /** The menu item opens a sub-menu. */
        SUBMENU
      };

      /** Stores the data associated with a menu item. */
      using Data =
        std::variant<ContextMenu*, Action, std::shared_ptr<BooleanModel>>;

      /**
       * Signals that a menu item was submitted.
       * @param menu The ContextMenu where the submission is triggered.
       * @param label The label of the submitted the menu item.
       */
      using SubmitSignal = Signal<
        void (const ContextMenu& menu, const QString& label)>;

      /**
       * The type of function used to make the QWidget associated with a menu
       * item.
       * @param type The type of menu item to make the widget for.
       * @param name The name of the item.
       * @param data The data associated with the item.
       */
      using ItemViewBuilder = std::function<QWidget* (
        MenuItemType type, const QString& name, const Data& data)>;

      /**
       * Constructs a ContextMenu.
       * @param parent The parent widget.
       */
      explicit ContextMenu(QWidget& parent);

      /**
       * Constructs a ContextMenu with a custom view builder.
       * @param parent The parent widget.
       * @param item_view_builder Makes the QWidget used to display menu items.
       */
      explicit ContextMenu(QWidget& parent, ItemViewBuilder item_view_builder);

      /**
       * Adds a menu.
       * @param name The name of the menu.
       * @param menu The menu to add.
       */
      void add_menu(const QString& name, ContextMenu& menu);

      /**
       * Adds an action.
       * @param name The name of the action.
       * @param action The action called when the item is submitted.
       */
      void add_action(const QString& name, const Action& action);

      /**
       * Adds an action.
       * @param name The name of the action.
       * @param action The action called when the item is submitted.
       * @param view The widget used to display the action.
       */
      void add_action(const QString& name, const Action& action, QWidget* view);

      /**
       * Adds an action that is disabled.
       * @param name The name to display.
       */
      void add_disabled_action(const QString& name);

      /**
       * Adds a CheckBox.
       * @param name The name of the menu item.
       * @return The checked state.
       */
      std::shared_ptr<BooleanModel> add_check_box(const QString& name);

      /**
       * Adds a CheckBox.
       * @param name The name of the menu item.
       * @param checked The checked state.
       */
      void add_check_box(const QString& name,
        const std::shared_ptr<BooleanModel>& checked);

      /** Adds a menu separator. */
      void add_separator();

      /** Resets the menu, removing all actions. */
      void reset();

      /**
       * Returns the menu item at a specified index, or <code>nullptr</code> iff
       * the index is out of range.
       */
      QWidget* get_menu_item(int index);

      /** Connects a slot to the Submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct MenuItem {
        int m_id;
        MenuItemType m_type;
        QString m_name;
        Data m_data;
      };
      mutable SubmitSignal m_submit_signal;
      ItemViewBuilder m_item_view_builder;
      int m_next_id;
      std::unordered_map<int, QWidget*> m_custom_views;
      std::shared_ptr<ArrayListModel<MenuItem>> m_list;
      ListView* m_list_view;
      OverlayPanel* m_window;
      OverlayPanel* m_visible_submenu;
      int m_pending_submenu_index;
      int m_hide_count;
      int m_last_show_items;
      QRect m_active_item_geometry;
      int m_block_move;
      MouseMoveObserver m_mouse_observer;
      QMargins m_window_border_size;
      boost::optional<QSize> m_window_size;
      std::unordered_map<int, OverlayPanel*> m_submenus;
      std::unordered_map<int, PressObserver> m_check_item_press_observers;
      boost::signals2::scoped_connection m_window_style_connection;

      QWidget* build_item(const std::shared_ptr<AnyListModel>& list, int index);
      ListItem* get_current_item() const;
      void clear_hover_style();
      void focus_first_item();
      void handle_right_or_enter_event(QEvent* event);
      bool handle_mouse_event(QMouseEvent* event);
      void position_submenu();
      void position_submenu(ListItem& item);
      bool is_submenu_hovered() const;
      void hide_submenu();
      void defer_hide_submenu();
      void show_submenu(int index);
      void on_mouse_move(QWidget& target, QMouseEvent& event);
      void on_list_operation(const ListModel<MenuItem>::Operation& operation);
      void on_submit(const std::any& submission);
      void on_window_style();
  };

  /**
   * Moves a ContextMenu under the cursor and shows it.
   * @param menu The ContextMenu to position under the cursor and show.
   */
  void show_under_cursor(ContextMenu& menu);

  /**
   * Adds an action to a ContextMenu displaying an icon along side it.
   * @param menu The menu to add the action to.
   * @param name The name of the action.
   * @param icon The icon to display.
   * @param action The action called when the item is submitted.
   */
  void add_action(ContextMenu& menu, const QString& name, QImage icon,
    const ContextMenu::Action& action);
}

#endif
