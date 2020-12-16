#ifndef NEXUS_QUERY_STANDARD_VALUES_HPP
#define NEXUS_QUERY_STANDARD_VALUES_HPP
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus::Queries {
  using QuantityValue = Beam::Queries::NativeValue<QuantityType>;
  using MoneyValue = Beam::Queries::NativeValue<MoneyType>;
  using SecurityValue = Beam::Queries::NativeValue<SecurityType>;
  using SecurityInfoValue = Beam::Queries::NativeValue<SecurityInfoType>;
  using OrderImbalanceValue = Beam::Queries::NativeValue<OrderImbalanceType>;
  using BboQuoteValue = Beam::Queries::NativeValue<BboQuoteType>;
  using BookQuoteValue = Beam::Queries::NativeValue<BookQuoteType>;
  using MarketQuoteValue = Beam::Queries::NativeValue<MarketQuoteType>;
  using TimeAndSaleValue = Beam::Queries::NativeValue<TimeAndSaleType>;
  using OrderFieldsValue = Beam::Queries::NativeValue<OrderFieldsType>;
  using OrderInfoValue = Beam::Queries::NativeValue<OrderInfoType>;
}

#endif
