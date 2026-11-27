#ifndef SPIRE_TABLE_AGGREGATOR_HPP
#define SPIRE_TABLE_AGGREGATOR_HPP
#include <compare>
#include <cstdint>
#include <functional>
#include <QString>
#include "Spire/Spire/AnyRef.hpp"

namespace Spire {

  /** The number of distinct values in an aggregate. */
  struct DistinctCount {
    std::int64_t m_value = 0;

    auto operator <=>(const DistinctCount&) const = default;
  };

  /** Formats a distinct count as "(n)". */
  QString to_text(DistinctCount count);

  /** Maintains the aggregate of one column in one group. */
  class TableAggregator {
    public:
      virtual ~TableAggregator() = default;

      /** Adds the value of a leaf entering the group. */
      virtual void add(const AnyRef& value) = 0;

      /** Removes the value of a leaf leaving the group. */
      virtual void remove(const AnyRef& value) = 0;

      /** Replaces the value contributed by a leaf in the group. */
      virtual void update(const AnyRef& previous, const AnyRef& current);

      /** Returns an owning snapshot of the current aggregate. */
      virtual std::any get() const = 0;

    protected:
      TableAggregator() = default;
      TableAggregator(TableAggregator&&) = default;

      TableAggregator& operator =(TableAggregator&&) = default;

    private:
      TableAggregator(const TableAggregator&) = delete;

      TableAggregator& operator =(const TableAggregator&) = delete;
  };

  /** Creates an independent, empty accumulator for a group. */
  using TableAggregatorFactory =
    std::function<std::unique_ptr<TableAggregator> ()>;

  /**
   * Sums numeric columns and counts distinct values in other supported columns.
   * Missing values are ignored, and an empty aggregate returns an empty any.
   */
  std::unique_ptr<TableAggregator> make_default_table_aggregator();
}

#endif
