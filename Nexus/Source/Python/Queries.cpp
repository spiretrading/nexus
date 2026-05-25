#include "Nexus/Python/Queries.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Queries/BboQuoteAccessor.hpp"
#include "Nexus/Queries/BookQuoteAccessor.hpp"
#include "Nexus/Queries/QuoteAccessor.hpp"
#include "Nexus/Queries/OrderImbalanceAccessor.hpp"
#include "Nexus/Queries/OrderFieldsAccessor.hpp"
#include "Nexus/Queries/OrderInfoAccessor.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"
#include "Nexus/Queries/TickerStatusAccessor.hpp"
#include "Nexus/Queries/TickerAccessor.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace pybind11;

void Nexus::Python::export_bbo_quote_accessor(module& module) {
  class_<BboQuoteAccessor>(module, "BboQuoteAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &BboQuoteAccessor::from_parameter).
    def_property_readonly("bid", &BboQuoteAccessor::get_bid).
    def_property_readonly("ask", &BboQuoteAccessor::get_ask).
    def_property_readonly("timestamp", &BboQuoteAccessor::get_timestamp);
}

void Nexus::Python::export_book_quote_accessor(module& module) {
  class_<BookQuoteAccessor>(module, "BookQuoteAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &BookQuoteAccessor::from_parameter).
    def_property_readonly("mpid", &BookQuoteAccessor::get_mpid).
    def_property_readonly(
      "is_primary_mpid", &BookQuoteAccessor::is_primary_mpid).
    def_property_readonly("venue", &BookQuoteAccessor::get_venue).
    def_property_readonly("quote", &BookQuoteAccessor::get_quote).
    def_property_readonly("timestamp", &BookQuoteAccessor::get_timestamp);
}

void Nexus::Python::export_order_fields_accessor(module& module) {
  class_<OrderFieldsAccessor>(module, "OrderFieldsAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &OrderFieldsAccessor::from_parameter).
    def_property_readonly("account", &OrderFieldsAccessor::get_account).
    def_property_readonly("ticker", &OrderFieldsAccessor::get_ticker).
    def_property_readonly("currency", &OrderFieldsAccessor::get_currency).
    def_property_readonly("type", &OrderFieldsAccessor::get_type).
    def_property_readonly("side", &OrderFieldsAccessor::get_side).
    def_property_readonly("destination", &OrderFieldsAccessor::get_destination).
    def_property_readonly("quantity", &OrderFieldsAccessor::get_quantity).
    def_property_readonly("price", &OrderFieldsAccessor::get_price).
    def_property_readonly(
      "time_in_force", &OrderFieldsAccessor::get_time_in_force);
}

void Nexus::Python::export_order_imbalance_accessor(module& module) {
  class_<OrderImbalanceAccessor>(module, "OrderImbalanceAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &OrderImbalanceAccessor::from_parameter).
    def_property_readonly("ticker", &OrderImbalanceAccessor::get_ticker).
    def_property_readonly("side", &OrderImbalanceAccessor::get_side).
    def_property_readonly("size", &OrderImbalanceAccessor::get_size).
    def_property_readonly(
      "reference_price", &OrderImbalanceAccessor::get_reference_price).
    def_property_readonly("timestamp", &OrderImbalanceAccessor::get_timestamp);
}

void Nexus::Python::export_order_info_accessor(module& module) {
  class_<OrderInfoAccessor>(module, "OrderInfoAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &OrderInfoAccessor::from_parameter).
    def_property_readonly("fields", &OrderInfoAccessor::get_fields).
    def_property_readonly(
      "submission_account", &OrderInfoAccessor::get_submission_account).
    def_property_readonly("order_id", &OrderInfoAccessor::get_order_id).
    def_property_readonly(
      "shorting_flag", &OrderInfoAccessor::get_shorting_flag).
    def_property_readonly("timestamp", &OrderInfoAccessor::get_timestamp).
    def_property_readonly("is_live", &OrderInfoAccessor::is_live);
}

void Nexus::Python::export_quote_accessor(module& module) {
  class_<QuoteAccessor>(module, "QuoteAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &QuoteAccessor::from_parameter).
    def_property_readonly("price", &QuoteAccessor::get_price).
    def_property_readonly("size", &QuoteAccessor::get_size).
    def_property_readonly("side", &QuoteAccessor::get_side);
}

void Nexus::Python::export_queries(module& module) {
  export_bbo_quote_accessor(module);
  export_book_quote_accessor(module);
  export_quote_accessor(module);
  export_order_fields_accessor(module);
  export_order_imbalance_accessor(module);
  export_order_info_accessor(module);
  export_ticker_accessor(module);
  export_ticker_status_accessor(module);
  export_time_and_sale_accessor(module);
  export_indexed_query<Ticker>(module, "TickerIndexedQuery");
  export_basic_query<Ticker>(module, "TickerQuery");
  export_indexed_query<Venue>(module, "VenueIndexedQuery");
  export_basic_query<Venue>(module, "VenueQuery");
  export_queue_suite<Nexus::QueryVariant>(module, "QueryVariant");
  export_queue_suite<Quantity>(module, "Quantity");
  export_queue_suite<Money>(module, "Money");
  export_queue_suite<Ticker>(module, "Ticker");
  export_queue_suite<Venue>(module, "Venue");
  export_queue_suite<TickerInfo>(module, "TickerInfo");
  export_queue_suite<TickerStatus>(module, "TickerStatus");
}

void Nexus::Python::export_ticker_accessor(module& module) {
  class_<TickerAccessor>(module, "TickerAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &TickerAccessor::from_parameter).
    def_property_readonly("symbol", &TickerAccessor::get_symbol).
    def_property_readonly("venue", &TickerAccessor::get_venue);
}

void Nexus::Python::export_ticker_status_accessor(module& module) {
  class_<TickerStatusAccessor>(module, "TickerStatusAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &TickerStatusAccessor::from_parameter).
    def_property_readonly("venue", &TickerStatusAccessor::get_venue).
    def_property_readonly("state", &TickerStatusAccessor::get_state).
    def_property_readonly("flags", &TickerStatusAccessor::get_flags).
    def_property_readonly("timestamp", &TickerStatusAccessor::get_timestamp);
}

void Nexus::Python::export_time_and_sale_accessor(module& module) {
  class_<TimeAndSaleAccessor>(module, "TimeAndSaleAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &TimeAndSaleAccessor::from_parameter).
    def_property_readonly("timestamp", &TimeAndSaleAccessor::get_timestamp).
    def_property_readonly("price", &TimeAndSaleAccessor::get_price).
    def_property_readonly("size", &TimeAndSaleAccessor::get_size).
    def_property_readonly(
      "market_center", &TimeAndSaleAccessor::get_market_center).
    def_property_readonly("buyer_mpid", &TimeAndSaleAccessor::get_buyer_mpid).
    def_property_readonly("seller_mpid", &TimeAndSaleAccessor::get_seller_mpid);
}

void Nexus::Python::export_value(module& module) {
  export_native_value<QuantityValue>(module, "QuantityValue");
  export_native_value<MoneyValue>(module, "MoneyValue");
  export_native_value<SideValue>(module, "SideValue");
  export_native_value<QuoteValue>(module, "QuoteValue");
  export_native_value<VenueValue>(module, "VenueValue");
  export_native_value<TickerValue>(module, "TickerValue");
  export_native_value<TickerInfoValue>(module, "TickerInfoValue");
  export_native_value<OrderImbalanceValue>(module, "OrderImbalanceValue");
  export_native_value<BboQuoteValue>(module, "BboQuoteValue");
  export_native_value<BookQuoteValue>(module, "BookQuoteValue");
  export_native_value<TickerStatusValue>(module, "TickerStatusValue");
  export_native_value<TimeAndSaleValue>(module, "TimeAndSaleValue");
  export_native_value<OrderFieldsValue>(module, "OrderFieldsValue");
  export_native_value<OrderInfoValue>(module, "OrderInfoValue");
}
