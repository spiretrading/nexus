#ifndef SPIRE_LIST_INDEX_TRACKER_HPP
#define SPIRE_LIST_INDEX_TRACKER_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /**
   * Keeps track of a list index in the face of updates such as deletions,
   * moves, and additions.
   */
  class ListIndexTracker {
    public:

      /**
       * Constructs a ListIndexTracker.
       * @param index The initial index to track.
       */
      explicit ListIndexTracker(int index);

      /**
       * Returns the current index being tracked, if the index was deleted then
       * the index will be <i>-1</i>.
       */
      int get_index() const;

      /** Sets the value of the index to track. */
      void set(int index);

      /**
       * Updates the index based on an operation.
       * @param operation The operation performed on the list.
       */
      void update(const AnyListModel::Operation& operation);

    private:
      int m_index;
  };
}

#endif
