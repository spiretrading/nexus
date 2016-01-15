#ifndef NEXUS_QUERYSTANDARDVALUES_HPP
#define NEXUS_QUERYSTANDARDVALUES_HPP
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
namespace Queries {
  using QuantityValue = Beam::Queries::NativeValue<QuantityType>;
  using MoneyValue = Beam::Queries::NativeValue<MoneyType>;
  using SecurityValue = Beam::Queries::NativeValue<SecurityType>;
  using OrderImbalanceValue = Beam::Queries::NativeValue<OrderImbalanceType>;
  using BboQuoteValue = Beam::Queries::NativeValue<BboQuoteType>;
  using BookQuoteValue = Beam::Queries::NativeValue<BookQuoteType>;
  using MarketQuoteValue = Beam::Queries::NativeValue<MarketQuoteType>;
  using TimeAndSaleValue = Beam::Queries::NativeValue<TimeAndSaleType>;
  using OrderFieldsValue = Beam::Queries::NativeValue<OrderFieldsType>;
  using OrderInfoValue = Beam::Queries::NativeValue<OrderInfoType>;
}
}

#endif
