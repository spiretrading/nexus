#ifndef SPIRE_TABLE_AGGREGATION_HPP
#define SPIRE_TABLE_AGGREGATION_HPP
#include <functional>
#include "Spire/Spire/AnyRef.hpp"

namespace Spire {

  /** Maintains the aggregate of one column in one group. */
  class TableAggregation {
    public:
      virtual ~TableAggregation() = default;

      /** Adds the value of a leaf entering the group. */
      virtual void add(const AnyRef& value) = 0;

      /** Removes the value of a leaf leaving the group. */
      virtual void remove(const AnyRef& value) = 0;

      /** Replaces the value contributed by a leaf in the group. */
      virtual void update(const AnyRef& previous, const AnyRef& current) = 0;

      /** Returns an owning snapshot of the current aggregate. */
      virtual std::any get() const = 0;
  };

  /** Creates an independent, empty accumulator for a group. */
  using TableAggregationFactory =
    std::function<std::unique_ptr<TableAggregation> ()>;

  /**
   * Makes a default accumulator. Numeric contributions are summed. If any
   * text is present, the result is the distinct display-value count "(n)".
   * Timestamps use their full ISO date and time for this count. Missing values
   * are ignored. An empty aggregate returns an empty std::any.
   */
  std::unique_ptr<TableAggregation> make_default_table_aggregation();
}

#endif
