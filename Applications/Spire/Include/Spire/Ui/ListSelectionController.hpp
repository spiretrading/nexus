#ifndef SPIRE_LIST_SELECTION_CONTROLLER_HPP
#define SPIRE_LIST_SELECTION_CONTROLLER_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Implements a controller managing a ListView's selection. */
  class ListSelectionController {
    public:

      /** Enumerates the different ways that items can be selected. */
      enum class Mode {

        /** Only a single item is selected. */
        SINGLE,

        /** Items are selected one by one. */
        INCREMENTAL,

        /** A range of items are selected at a time. */
        RANGE
      };

      /**
       * Constructs a ListSelectionController over a given selection model with
       * an initial selection mode set to SINGLE.
       * @param selection The model of selected items.
       */
      explicit ListSelectionController(
        std::shared_ptr<ListModel<int>> selection);

      /** Returns the selection model. */
      const std::shared_ptr<ListModel<int>>& get_selection() const;

      /** Sets the selection mode. */
      void set_mode(Mode mode);

      /**
       * Updates the selection when an item is added to the list.
       * @param index The index of the added item.
       */
      void add(int index);

      /**
       * Updates the selection when an item is removed from the list.
       * @param index The index of the removed item.
       */
      void remove(int index);

      /**
       * Updates the selection when an item is moved.
       * @param source The index where the item was moved from.
       * @param destination The index that the item was moved to.
       */
      void move(int source, int destination);

      /**
       * Updates the selection when an item is clicked.
       * @param The index of the clicked item.
       */
      void click(int index);

      /**
       * Updates the selection when an item is navigated to (by keyboard).
       * @param The index of item that was navigated to.
       */
      void navigate(int index);

      /** Connects a slot to a signal indicating a change to the selection. */
      boost::signals2::connection connect_operation_signal(
        const ListModel<int>::OperationSignal::slot_type& slot) const;

    private:
      mutable ListModel<int>::OperationSignal m_operation_signal;
      Mode m_mode;
      std::shared_ptr<ListModel<int>> m_selection;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const ListModel<int>::Operation& operation);
  };
}

#endif
