#include "Spire/TimeAndSales/BboIndicator.hpp"
#include <QObject>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"

using namespace Nexus;
using namespace Spire;

BboIndicator Spire::get_bbo_indicator(
    const BboQuote& bbo, const TimeAndSale& time_and_sale) {
  if(bbo.m_ask.m_price == Money::ZERO) {
    return BboIndicator::UNKNOWN;
  } else if(time_and_sale.m_price == bbo.m_bid.m_price) {
    return BboIndicator::AT_BID;
  } else if(time_and_sale.m_price < bbo.m_bid.m_price) {
    return BboIndicator::BELOW_BID;
  } else if(time_and_sale.m_price == bbo.m_ask.m_price) {
    return BboIndicator::AT_ASK;
  } else if(time_and_sale.m_price > bbo.m_ask.m_price) {
    return BboIndicator::ABOVE_ASK;
  }
  return BboIndicator::INSIDE;
}

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
