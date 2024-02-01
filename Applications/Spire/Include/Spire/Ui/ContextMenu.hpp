#ifndef SPIRE_CONTEXT_MENU_HPP
#define SPIRE_CONTEXT_MENU_HPP
#include <variant>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a pop-up context menu. */
  class ContextMenu : public QWidget {
    public:

      /**
       * Signals that a menu item was submitted.
       * @param menu The ContextMenu where the submission is triggered.
       * @param label The label of the submitted the menu item.
       */
      using SubmitSignal = Signal<void (const ContextMenu& menu,
        const QString& label)>;

      /**
       * The type of function used to do an action.
       */
      using Action = std::function<void ()>;

      /**
       * Constructs a ContextMenu.
       * @param parent The parent widget.
       */
      explicit ContextMenu(QWidget& parent);

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

      /** Connects a slot to the Submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      enum class MenuItemType {
        ACTION,
        CHECK,
        SEPARATOR,
        SUBMENU
      };
      struct MenuItem {
        MenuItemType m_type;
        QString m_name;
        std::variant<ContextMenu*, Action, std::shared_ptr<BooleanModel>>
          m_data;
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ArrayListModel<MenuItem>> m_list;
      ListView* m_list_view;
      OverlayPanel* m_window;
      QWidget* m_active_menu_window;
      QMargins m_window_border_size;
      boost::optional<QSize> m_window_size;
      std::unordered_map<int, QWidget*> m_submenus;
      boost::signals2::scoped_connection m_window_style_connection;

      QWidget* build_item(const std::shared_ptr<AnyListModel>& list, int index);
      ListItem* get_current_item() const;
      void clear_hover_style();
      void focus_first_item();
      void handle_right_or_enter_event(QEvent* event);
      bool handle_mouse_event(QMouseEvent* event);
      void position_menu(ListItem* item);
      void hide_active_menu();
      void show_submenu(int index);
      void on_list_operation(const ListModel<MenuItem>::Operation& operation);
      void on_submit(const std::any& submission);
      void on_window_style();
  };
}

#endif
