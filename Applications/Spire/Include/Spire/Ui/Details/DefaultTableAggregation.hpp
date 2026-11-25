#ifndef SPIRE_DEFAULT_TABLE_AGGREGATION_HPP
#define SPIRE_DEFAULT_TABLE_AGGREGATION_HPP
#include "Spire/Ui/TableAggregation.hpp"

namespace Spire::Details {
  using TableValueVisitor = std::function<void (const AnyRef&)>;
  std::unique_ptr<TableAggregation> make_default_table_aggregation(
    std::function<void (const TableValueVisitor&)> visit);
}

#endif
