#ifndef NEXUS_SHUTTLE_QUERY_TYPES_HPP
#define NEXUS_SHUTTLE_QUERY_TYPES_HPP
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

namespace Nexus {
  BEAM_REGISTER_TYPES(register_value_types,
    (BboQuoteValue, "Nexus.Queries.BboQuoteValue"),
    (BookQuoteValue, "Nexus.Queries.BookQuoteValue"),
    (MoneyValue, "Nexus.Queries.MoneyValue"),
    (OrderFieldsValue, "Nexus.Queries.OrderFieldsValue"),
    (OrderImbalanceValue, "Nexus.Queries.OrderImbalanceValue"),
    (OrderInfoValue, "Nexus.Queries.OrderInfoValue"),
    (QuantityValue, "Nexus.Queries.QuantityValue"),
    (QuoteValue, "Nexus.Queries.QuoteValue"),
    (SideValue, "Nexus.Queries.SideValue"),
    (TickerInfoValue, "Nexus.Queries.TickerInfoValue"),
    (TickerStatusValue, "Nexus.Queries.TickerStatusValue"),
    (TickerValue, "Nexus.Queries.TickerValue"),
    (TimeAndSaleValue, "Nexus.Queries.TimeAndSaleValue"),
    (VenueValue, "Nexus.Queries.VenueValue"));

  template<Beam::IsSender S>
  void register_query_types(Beam::Out<Beam::TypeRegistry<S>> registry) {
    Beam::register_query_types(out(registry));
    registry->add(typeid(BboQuote), "Nexus.BboQuote");
    registry->add(typeid(BookQuote), "Nexus.BookQuote");
    registry->add(typeid(Money), "Nexus.Money");
    registry->add(typeid(OrderFields), "Nexus.OrderFields");
    registry->add(typeid(OrderImbalance), "Nexus.OrderImbalance");
    registry->add(typeid(OrderInfo), "Nexus.OrderInfo");
    registry->add(typeid(Quantity), "Nexus.Quantity");
    registry->add(typeid(Quote), "Nexus.Quote");
    registry->add(typeid(Side), "Nexus.Side");
    registry->add(typeid(Ticker), "Nexus.Ticker");
    registry->add(typeid(TickerInfo), "Nexus.TickerInfo");
    registry->add(typeid(TickerStatus), "Nexus.TickerStatus");
    registry->add(typeid(TimeAndSale), "Nexus.TimeAndSale");
    registry->add(typeid(Venue), "Nexus.Venue");
    Nexus::register_value_types(out(registry));
  }
}

#endif
