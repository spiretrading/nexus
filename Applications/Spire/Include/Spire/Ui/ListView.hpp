#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListModel.hpp"

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

      /**
       * A ValueModel over an optional QString to represent the current of the
       * list view.
       */
      using CurrentModel = ValueModel<boost::optional<QString>>;

      /**
       * A LcoalValueModel over an optional QString to represent the local
       * current of the list view.
       */
      using LocalCurrentModel = LocalValueModel<boost::optional<QString>>;

      /**
       * Signals the user has clicked to delete a list item.
       * @param value The value associated with the item being requested for
       *              deletion. 
       */
      using DeleteSignal = Signal<void (const QString& value)>;
  
      /**
       * Signals that the item was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QString& submission)>;
  
      /**
       * Constructs a ListView.
       * @param current_model The current value's model .
       * @param list_model The list model which holds the a list of items. 
       * @param factory A function that takes a ListMoel and a index
       *                used to construct a widget displayed in the ListView.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<CurrentModel> current_model,
        std::shared_ptr<ListModel> list_model,
        std::function<QWidget* (std::shared_ptr<ListModel>, int index)> factory,
        QWidget* parent = nullptr);
  
      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /** Returns the list model. */
      const std::shared_ptr<ListModel>& get_list_model() const;

      /** Returns the direction of the list. */
      Qt::Orientation get_direction() const;

      /** Sets the direction of the list. */
      void set_direction(Qt::Orientation direction);
      
      /** Returns the navigation behavior of the list. */
      EdgeNavigation get_edge_navigation() const;

      /** Sets the navigation behavior of the list. */
      void set_edge_navigation(EdgeNavigation navigation);

      /** Returns the overflow mode of the list. */
      Overflow get_overflow() const;

      /** Sets the overflow mode of the list. */
      void set_overflow(Overflow overflow);

      /** Connects a slot to the delete signal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;
  
      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable DeleteSignal m_delete_signal;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<CurrentModel> m_current_model;
      std::shared_ptr<ListModel> m_list_model;
      std::function<QWidget* (std::shared_ptr<ListModel>, int index)> m_factory;
      Qt::Orientation m_direction;
      EdgeNavigation m_navigation;
      Overflow m_overflow;
      std::vector<QWidget*> m_items;
      int m_gap;
      int m_overflow_gap;

      void update_layout();
  };
}

#endif
