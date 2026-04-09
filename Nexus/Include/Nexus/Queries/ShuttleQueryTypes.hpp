#ifndef NEXUS_SHUTTLE_QUERY_TYPES_HPP
#define NEXUS_SHUTTLE_QUERY_TYPES_HPP
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus {
  BEAM_REGISTER_TYPES(register_value_types,
    (QuantityValue, "Nexus.Queries.QuantityValue"),
    (TickerValue, "Nexus.Queries.TickerValue"),
    (TickerInfoValue, "Nexus.Queries.TickerInfoValue"),
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
    registry->add(typeid(Ticker), "Nexus.Ticker");
    registry->add(typeid(TickerInfo), "Nexus.TickerInfo");
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
