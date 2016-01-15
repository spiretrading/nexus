#ifndef NEXUS_SHUTTLEQUERYTYPES_HPP
#define NEXUS_SHUTTLEQUERYTYPES_HPP
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus {
namespace Queries {
  BEAM_REGISTER_TYPES(RegisterDataTypes,
    (QuantityType, "Nexus.Queries.QuantityType"),
    (MoneyType, "Nexus.Queries.MoneyType"),
    (SecurityType, "Nexus.Queries.SecurityType"),
    (OrderImbalanceType, "Nexus.Queries.OrderImbalanceType"),
    (BboQuoteType, "Nexus.Queries.BboQuoteType"),
    (BookQuoteType, "Nexus.Queries.BookQuoteType"),
    (MarketQuoteType, "Nexus.Queries.MarketQuoteType"),
    (TimeAndSaleType, "Nexus.Queries.TimeAndSaleType"),
    (OrderFieldsType, "Nexus.Queries.OrderFieldsType"),
    (OrderInfoType, "Nexus.Queries.OrderInfoType"));

  BEAM_REGISTER_TYPES(RegisterValueTypes,
    (QuantityValue, "Nexus.Queries.QuantityValue"),
    (SecurityValue, "Nexus.Queries.SecurityValue"),
    (MoneyValue, "Nexus.Queries.MoneyValue"),
    (OrderImbalanceValue, "Nexus.Queries.OrderImbalanceValue"),
    (BboQuoteValue, "Nexus.Queries.BboQuoteValue"),
    (BookQuoteValue, "Nexus.Queries.BookQuoteValue"),
    (MarketQuoteValue, "Nexus.Queries.MarketQuoteValue"),
    (TimeAndSaleValue, "Nexus.Queries.TimeAndSaleValue"),
    (OrderFieldsValue, "Nexus.Queries.OrderFieldsValue"),
    (OrderInfoValue, "Nexus.Queries.OrderInfoValue"));

  template<typename SenderType>
  void RegisterQueryTypes(Beam::Out<
      Beam::Serialization::TypeRegistry<SenderType>> registry) {
    Beam::Queries::RegisterQueryTypes(Beam::Store(registry));
    RegisterDataTypes(Beam::Store(registry));
    RegisterValueTypes(Beam::Store(registry));
  }
}
}

#endif
