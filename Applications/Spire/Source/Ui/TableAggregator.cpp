#include "Spire/Ui/TableAggregator.hpp"

using namespace Spire;

void TableAggregator::update(const AnyRef& previous, const AnyRef& current) {
  remove(previous);
  add(current);
}
