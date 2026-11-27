#include "Spire/Ui/TableAggregator.hpp"
#include "Spire/Ui/DefaultTableAggregator.hpp"

using namespace Spire;

QString Spire::to_text(DistinctCount count) {
  return QStringLiteral("(%1)").arg(count.m_value);
}

void TableAggregator::update(const AnyRef& previous, const AnyRef& current) {
  remove(previous);
  add(current);
}

std::unique_ptr<TableAggregator> Spire::make_default_table_aggregator() {
  return std::make_unique<DefaultTableAggregator>();
}
