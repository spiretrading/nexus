#ifndef NEXUS_QUERY_STANDARD_VALUES_HPP
#define NEXUS_QUERY_STANDARD_VALUES_HPP
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
  using QuantityValue = Beam::NativeValue<Quantity>;
  using MoneyValue = Beam::NativeValue<Money>;
  using SideValue = Beam::NativeValue<Side>;
  using QuoteValue = Beam::NativeValue<Quote>;
  using TickerValue = Beam::NativeValue<Ticker>;
  using TickerInfoValue = Beam::NativeValue<TickerInfo>;
  using OrderImbalanceValue = Beam::NativeValue<OrderImbalance>;
  using BboQuoteValue = Beam::NativeValue<BboQuote>;
  using BookQuoteValue = Beam::NativeValue<BookQuote>;
  using TickerStatusValue = Beam::NativeValue<TickerStatus>;
  using TimeAndSaleValue = Beam::NativeValue<TimeAndSale>;
  using OrderFieldsValue = Beam::NativeValue<OrderFields>;
  using OrderInfoValue = Beam::NativeValue<OrderInfo>;
}

#endif
