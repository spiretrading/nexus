#ifndef NEXUS_SHUTTLE_QUERY_TYPES_HPP
#define NEXUS_SHUTTLE_QUERY_TYPES_HPP
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus {
  BEAM_REGISTER_TYPES(register_value_types,
    (QuantityValue, "Nexus.Queries.QuantityValue"),
    (SecurityValue, "Nexus.Queries.SecurityValue"),
    (SecurityInfoValue, "Nexus.Queries.SecurityInfoValue"),
    (MoneyValue, "Nexus.Queries.MoneyValue"),
    (OrderImbalanceValue, "Nexus.Queries.OrderImbalanceValue"),
    (BboQuoteValue, "Nexus.Queries.BboQuoteValue"),
    (BookQuoteValue, "Nexus.Queries.BookQuoteValue"),
    (TimeAndSaleValue, "Nexus.Queries.TimeAndSaleValue"),
    (OrderFieldsValue, "Nexus.Queries.OrderFieldsValue"),
    (OrderInfoValue, "Nexus.Queries.OrderInfoValue"));

  template<Beam::IsSender S>
  void register_query_types(Beam::Out<Beam::TypeRegistry<S>> registry) {
    Beam::register_query_types(out(registry));
    registry->add(typeid(Money), "Nexus.Money");
    registry->add(typeid(Quantity), "Nexus.Quantity");
    registry->add(typeid(Security), "Nexus.Securty");
    registry->add(typeid(SecurityInfo), "Nexus.SecurtyInfo");
    registry->add(typeid(OrderImbalance), "Nexus.OrderImbalance");
    registry->add(typeid(BboQuote), "Nexus.BboQuote");
    registry->add(typeid(BookQuote), "Nexus.BookQuote");
    registry->add(typeid(TimeAndSale), "Nexus.TimeAndSale");
    registry->add(typeid(OrderFields), "Nexus.OrderFields");
    registry->add(typeid(OrderInfo), "Nexus.OrderInfo");
    Nexus::register_value_types(out(registry));
  }
}

#endif
