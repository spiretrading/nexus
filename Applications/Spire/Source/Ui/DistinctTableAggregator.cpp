#include "Spire/Ui/DistinctTableAggregator.hpp"

using namespace Spire;

QString Spire::to_text(DistinctCount count) {
  return QStringLiteral("(%1)").arg(count.m_value);
}
