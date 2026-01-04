#include "Nexus/Python/Queries.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Queries/OrderFieldsAccessor.hpp"
#include "Nexus/Queries/OrderInfoAccessor.hpp"
#include "Nexus/Queries/SecurityAccessor.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace pybind11;

void Nexus::Python::export_order_fields_accessor(module& module) {
  class_<OrderFieldsAccessor>(module, "OrderFieldsAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &OrderFieldsAccessor::from_parameter).
    def_property_readonly("account", &OrderFieldsAccessor::get_account).
    def_property_readonly("security", &OrderFieldsAccessor::get_security).
    def_property_readonly("currency", &OrderFieldsAccessor::get_currency).
    def_property_readonly("type", &OrderFieldsAccessor::get_type).
    def_property_readonly("side", &OrderFieldsAccessor::get_side).
    def_property_readonly("destination", &OrderFieldsAccessor::get_destination).
    def_property_readonly("quantity", &OrderFieldsAccessor::get_quantity).
    def_property_readonly("price", &OrderFieldsAccessor::get_price).
    def_property_readonly(
      "time_in_force", &OrderFieldsAccessor::get_time_in_force);
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

void Nexus::Python::export_queries(module& module) {
  export_order_fields_accessor(module);
  export_order_info_accessor(module);
  export_security_accessor(module);
  export_time_and_sale_accessor(module);
  export_indexed_query<Security>(module, "SecurityIndexedQuery");
  export_basic_query<Security>(module, "SecurityQuery");
  export_indexed_query<Venue>(module, "VenueIndexedQuery");
  export_basic_query<Venue>(module, "VenueQuery");
  export_queue_suite<Nexus::QueryVariant>(module, "QueryVariant");
  export_queue_suite<Quantity>(module, "Quantity");
  export_queue_suite<Money>(module, "Money");
  export_queue_suite<Security>(module, "Security");
  export_queue_suite<Venue>(module, "Venue");
  export_queue_suite<SecurityInfo>(module, "SecurityInfo");
}

void Nexus::Python::export_security_accessor(module& module) {
  class_<SecurityAccessor>(module, "SecurityAccessor").
    def(init<Expression>()).
    def_static("from_parameter", &SecurityAccessor::from_parameter).
    def_property_readonly("symbol", &SecurityAccessor::get_symbol).
    def_property_readonly("venue", &SecurityAccessor::get_venue);
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
  export_native_value<SecurityValue>(module, "SecurityValue");
  export_native_value<SecurityInfoValue>(module, "SecurityInfoValue");
  export_native_value<OrderImbalanceValue>(module, "OrderImbalanceValue");
  export_native_value<BboQuoteValue>(module, "BboQuoteValue");
  export_native_value<BookQuoteValue>(module, "BookQuoteValue");
  export_native_value<TimeAndSaleValue>(module, "TimeAndSaleValue");
  export_native_value<OrderFieldsValue>(module, "OrderFieldsValue");
  export_native_value<OrderInfoValue>(module, "OrderInfoValue");
}
