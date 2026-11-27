#ifndef SPIRE_TABLE_AGGREGATOR_HPP
#define SPIRE_TABLE_AGGREGATOR_HPP
#include "Spire/Spire/AnyRef.hpp"

namespace Spire {

  /** Maintains the aggregate of one column in one group. */
  class TableAggregator {
    public:
      virtual ~TableAggregator() = default;

      /**
       * Adds the contribution of a leaf entering the group.
       * @param value The leaf's value in the aggregated column.
       */
      virtual void add(const AnyRef& value) = 0;

      /**
       * Removes the contribution of a leaf leaving the group.
       * @param value The leaf's last value in the aggregated column.
       */
      virtual void remove(const AnyRef& value) = 0;

      /**
       * Replaces the contribution of a leaf in the group.
       * @param previous The leaf's value in the aggregated column before the
       *        update.
       * @param current The leaf's value in that column after the update.
       */
      virtual void update(const AnyRef& previous, const AnyRef& current);

      /** Returns an owning snapshot of the current aggregate. */
      virtual std::any get() const = 0;

    protected:
      TableAggregator() = default;

    private:
      TableAggregator(const TableAggregator&) = delete;
      TableAggregator& operator =(const TableAggregator&) = delete;
  };
}

#endif
