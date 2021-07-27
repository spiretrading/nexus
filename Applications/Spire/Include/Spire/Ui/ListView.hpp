#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <QLayout>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListItem.hpp"

namespace Spire {
namespace Styles {

  /** Sets the spacing between list items. */
  using ListItemGap = BasicProperty<int, struct ListItemGapTag>;

  /**
   * Sets the gap between list items on overflow=WRAP in direction perpendicular
   * to list direction.
   */
  using ListOverflowGap = BasicProperty<int, struct ListOverflowGapTag>;
}

  /**
   * Represents a list view which contains a vertical or horizontal stack of
   * components.
   */
  class ListView : public QWidget {
    public:

      /** 
       * Specifies the keyboard navigation behavior when the first or last list
       * item is selected and the key for next or previous list item is pressed.
       */
      enum class EdgeNavigation {

        /** Selection stops at the current selection. */
        CONTAIN,

        /** Selection moves from the first item to last item and vice versa. */
        WRAP
      };

      /** Specifies how to layout items on overflow. */
      enum class Overflow {

        /** The list extends indefinitely. */
        NONE,

        /** List items wrap to fill the perpendicular space. */
        WRAP
      };

      /** Specifies the selection behavior for the ListView. */
      enum class SelectionMode {

        /** Items can not be selected. */
        NONE,

        /** The user can select a single item. */
        SINGLE
      };

      /**
       * A ValueModel over an optional std::any to represent the current of the
       * list view.
       */
      using CurrentModel = ValueModel<boost::optional<std::any>>;

      /**
       * A LocalValueModel over an optional std::any to represent the local
       * current of the list view.
       */
      using LocalCurrentModel = LocalValueModel<boost::optional<std::any>>;

      /**
       * Signals that the item was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;
  
      /**
       * Constructs a ListView.
       * @param current_model The current value's model.
       * @param list_model The list model which holds a list of items.
       * @param factory A function that takes a ListModel and a index
       *                used to construct a widget displayed in the ListView.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<CurrentModel> current_model,
        std::shared_ptr<ArrayListModel> list_model,
        std::function<QWidget* (
          std::shared_ptr<ArrayListModel>, int index)> factory,
        QWidget* parent = nullptr);
  
      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /** Returns the list model. */
      const std::shared_ptr<ArrayListModel>& get_list_model() const;

      /** Returns the direction of the ListView. */
      Qt::Orientation get_direction() const;

      /** Sets the direction of the ListView. */
      void set_direction(Qt::Orientation direction);
      
      /** Returns the navigation behavior of the ListView. */
      EdgeNavigation get_edge_navigation() const;

      /**
       * Sets the navigation behavior of the ListView.
       * @param navigation The keyboard navigation behavior when the first or
       *                    last list item is selected.
       */
      void set_edge_navigation(EdgeNavigation navigation);

      /** Returns the overflow mode of the ListView. */
      Overflow get_overflow() const;

      /**
       * Sets the overflow mode of the ListView.
       * @param overflow The overflow model.
       */
      void set_overflow(Overflow overflow);

      /** Returns the selection mode of the ListView. */
      SelectionMode get_selection_mode() const;

      /**
       * Sets the selection mode of the ListView.
       * @param selection_mode The selection mode for the ListView.
       */
      void set_selection_mode(SelectionMode selection_mode);

      /**
       * Returns whether items are selected when focused for
       * selection_mode = SINGLE.
       */
      bool does_selection_follow_focus() const;

      /**
       * Sets whether items are selected when focused for
       * selection_mode = SINGLE.
       * @param does_selection_follow_focus True iff items are selected
       *                                   when focused
       */
      void set_selection_follow_focus(bool does_selection_follow_focus);

      /** Returns the value of the selected list item. */
      const std::any& get_selected() const;

      /**
       * Returns the ListItem connected the specified value.
       * @param value The value associated with an item.
       */
      ListItem* get_list_item(const std::any& value) const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct Item {
        ListItem* m_item;
        boost::signals2::scoped_connection m_current_connection;
        boost::signals2::scoped_connection m_submit_connection;
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<CurrentModel> m_current_model;
      std::shared_ptr<ArrayListModel> m_list_model;
      std::function<QWidget* (
        std::shared_ptr<ArrayListModel>, int index)> m_factory;
      Qt::Orientation m_direction;
      EdgeNavigation m_navigation;
      Overflow m_overflow;
      SelectionMode m_selection_mode;
      bool m_does_selection_follow_focus;
      std::any m_selected;
      std::vector<Item> m_items;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_list_model_connection;
      int m_current_index;
      int m_column_or_row_index;
      QPoint m_tracking_position;
      bool m_is_setting_item_focus;
      QString m_query;
      QTimer m_query_timer;
      QWidget* m_body;

      boost::signals2::scoped_connection connect_item_current(ListItem* item,
        const std::any& value);
      boost::signals2::scoped_connection connect_item_submit(ListItem* item,
        const std::any& value);
      int get_index_by_value(const std::any& value) const;
      QLayout* get_layout();
      QLayoutItem* get_column_or_row(int index);
      void select_item(bool is_selected);
      void cross_move(bool is_next);
      int move_next();
      int move_previous();
      void on_current(const boost::optional<std::any>& current);
      void on_operation(const ListModel::Operation& operation);
      void on_add_item(int index);
      void on_delete_item(int index);
      void update_column_row_index();
      void update_layout();
      void update_tracking_position();
      void update_current(int index, bool is_update_x_y);
      void update_current(int index);
      void update_after_items_changed();
      void update_selection(const std::any& selected);
      void update_item_size_policy(ListItem& item);
      void query();
  };
}

#endif
