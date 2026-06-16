#include "Spire/TimeAndSales/BboIndicator.hpp"
#include <QObject>

using namespace Spire;

const QString& Spire::to_text(BboIndicator indicator) {
  if(indicator == BboIndicator::ABOVE_ASK) {
    static const auto value = QObject::tr("Above Ask");
    return value;
  } else if(indicator == BboIndicator::AT_ASK) {
    static const auto value = QObject::tr("At Ask");
    return value;
  } else if(indicator == BboIndicator::INSIDE) {
    static const auto value = QObject::tr("Inside");
    return value;
  } else if(indicator == BboIndicator::AT_BID) {
    static const auto value = QObject::tr("At Bid");
    return value;
  } else if(indicator == BboIndicator::BELOW_BID) {
    static const auto value = QObject::tr("Below Bid");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}
