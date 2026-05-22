#ifndef NEXUS_QUERY_STANDARD_VALUES_HPP
#define NEXUS_QUERY_STANDARD_VALUES_HPP
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
  using BboQuoteValue = Beam::NativeValue<BboQuote>;
  using BookQuoteValue = Beam::NativeValue<BookQuote>;
  using MoneyValue = Beam::NativeValue<Money>;
  using OrderFieldsValue = Beam::NativeValue<OrderFields>;
  using OrderImbalanceValue = Beam::NativeValue<OrderImbalance>;
  using OrderInfoValue = Beam::NativeValue<OrderInfo>;
  using QuantityValue = Beam::NativeValue<Quantity>;
  using QuoteValue = Beam::NativeValue<Quote>;
  using SideValue = Beam::NativeValue<Side>;
  using TickerInfoValue = Beam::NativeValue<TickerInfo>;
  using TickerStatusValue = Beam::NativeValue<TickerStatus>;
  using TickerValue = Beam::NativeValue<Ticker>;
  using TimeAndSaleValue = Beam::NativeValue<TimeAndSale>;
  using VenueValue = Beam::NativeValue<Venue>;
}

#endif
