#ifndef NEXUS_QUERY_STANDARD_VALUES_HPP
#define NEXUS_QUERY_STANDARD_VALUES_HPP
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
  using QuantityValue = Beam::NativeValue<Quantity>;
  using MoneyValue = Beam::NativeValue<Money>;
  using SecurityValue = Beam::NativeValue<Security>;
  using SecurityInfoValue = Beam::NativeValue<SecurityInfo>;
  using OrderImbalanceValue = Beam::NativeValue<OrderImbalance>;
  using BboQuoteValue = Beam::NativeValue<BboQuote>;
  using BookQuoteValue = Beam::NativeValue<BookQuote>;
  using TimeAndSaleValue = Beam::NativeValue<TimeAndSale>;
  using OrderFieldsValue = Beam::NativeValue<OrderFields>;
  using OrderInfoValue = Beam::NativeValue<OrderInfo>;
}

#endif
