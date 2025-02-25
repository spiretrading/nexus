#ifndef SPIRE_LIST_CURRENT_CONTROLLER_HPP
#define SPIRE_LIST_CURRENT_CONTROLLER_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QRect>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Implements a controller managing a ListView's current index. */
  class ListCurrentController {
    public:

      /**
       * Specifies the navigation behavior when the first or last list item is
       * selected and the key for next or previous list item is pressed.
       */
      enum class EdgeNavigation {

        /** Selection stops at the current selection. */
        CONTAIN,

        /** Selection moves from the first item to last item and vice versa. */
        WRAP
      };

      /** Provides an abstract representation of a list item's view. */
      class ItemView {
        public:
          virtual ~ItemView() = default;

          /**
           * Returns <code>true</code> iff this item can be selected/navigated
           * to.
           */
          virtual bool is_selectable() const = 0;

          /** Returns the item's position/size. */
          virtual QRect get_geometry() const = 0;
      };

      /** The type of model representing the index of the current value. */
      using CurrentModel = ValueModel<boost::optional<int>>;

      /**
       * Signals a change to the current navigation index.
       * @param current The index of the current navigation index.
       */
      using UpdateSignal = Signal<void (const boost::optional<int>& current)>;

      /**
       * Constructs a ListCurrentController over a given current index model
       * with an EdgeNavigation set to WRAP.
       * @param current The current index model.
       * @param size The size of the list.
       */
      ListCurrentController(std::shared_ptr<CurrentModel> current, int size);

      /** Returns the current index model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns how edge navigation is handled. */
      EdgeNavigation get_edge_navigation() const;

      /** Sets the way edge navigation is handled. */
      void set_edge_navigation(EdgeNavigation navigation);

      /**
       * Updates the controller with a newly added item.
       * @param view The item's view.
       * @param index The index of the added item.
       */
      void add(std::unique_ptr<ItemView> view, int index);

      /**
       * Updates the controller with a removed item.
       * @param index The index of the removed item.
       */
      void remove(int index);

      /**
       * Updates the controller when an item is moved.
       * @param source The index where the item was moved from.
       * @param destination The index that the item was moved to.
       */
      void move(int source, int destination);

      /** Sets the current to the first selectable item. */
      void navigate_home();

      /** Sets the current to the last selectable item. */
      void navigate_end();

      /** Sets the current to the next (or first) selectable item. */
      void navigate_next();

      /** Sets the current to the previous (or first) selectable item. */
      void navigate_previous();

      /**
       * Navigates in a specified direction.
       * @param direction The direction to navigate.
       * @param start The index of the item to navigate from.
       */
      void navigate(int direction, int start);

      /**
       * Navigates in a specified direction.
       * @param direction The direction to navigate.
       * @param start The index of the item to navigate from.
       * @param edge_navigation How to handle edge navigation.
       */
      void navigate(int direction, int start, EdgeNavigation edge_navigation);

      /**
       * Cross navigates to the next (or first) selectable item.
       * @param orientation The orientation of the items layout.
       */
      void cross_next(Qt::Orientation orientation);

      /**
       * Cross navigates to the last (or first) selectable item.
       * @param orientation The orientation of the items layout.
       */
      void cross_previous(Qt::Orientation orientation);

      /**
       * Cross navigates in a specified direction.
       * @param direction The direction to cross navigate.
       * @param orientation The orientation of the items layout.
       */
      void cross(int direction, Qt::Orientation orientation);

      /**
       * Connects a slot to a signal indicating a change to the current index.
       */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      std::vector<std::unique_ptr<ItemView>> m_views;
      std::shared_ptr<CurrentModel> m_current;
      int m_size;
      EdgeNavigation m_edge_navigation;
      boost::signals2::scoped_connection m_connection;

      bool is_initialized() const;
      void on_current(boost::optional<int> current);
  };
}

#endif
