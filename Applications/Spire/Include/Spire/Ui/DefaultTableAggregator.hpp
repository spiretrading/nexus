#ifndef SPIRE_DEFAULT_TABLE_AGGREGATOR_HPP
#define SPIRE_DEFAULT_TABLE_AGGREGATOR_HPP
#include "Spire/Ui/TableAggregator.hpp"

namespace Spire {

  /**
   * Sums numeric values and counts distinct values of other supported types.
   */
  class DefaultTableAggregator final : public TableAggregator {
    public:

      /** Constructs an empty accumulator. */
      DefaultTableAggregator() = default;

      void add(const AnyRef& value) override;
      void remove(const AnyRef& value) override;
      void update(const AnyRef& previous, const AnyRef& current) override;
      std::any get() const override;

    private:
      std::unique_ptr<TableAggregator> m_aggregator;

      void select_aggregator(const AnyRef& value);
  };
}

#endif
