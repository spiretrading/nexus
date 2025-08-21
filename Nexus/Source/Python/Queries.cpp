#include "Nexus/Python/Queries.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::Queries;
using namespace pybind11;

void Nexus::Python::export_data_type(module& module) {
  ExportNativeDataType<QuantityType>(module, "QuantityType");
  ExportNativeDataType<MoneyType>(module, "MoneyType");
  ExportNativeDataType<SecurityType>(module, "SecurityType");
  ExportNativeDataType<SecurityInfoType>(module, "SecurityInfoType");
  ExportNativeDataType<OrderImbalanceType>(module, "OrderImbalanceType");
  ExportNativeDataType<BboQuoteType>(module, "BboQuoteType");
  ExportNativeDataType<BookQuoteType>(module, "BookQuoteType");
  ExportNativeDataType<TimeAndSaleType>(module, "TimeAndSaleType");
  ExportNativeDataType<OrderFieldsType>(module, "OrderFieldsType");
  ExportNativeDataType<OrderInfoType>(module, "OrderInfoType");
}

void Nexus::Python::export_queries(module& module) {
  auto submodule = module.def_submodule("queries");
  export_time_and_sale_accessor(submodule);
  ExportIndexedQuery<Security>(submodule, "SecurityIndexedQuery");
  ExportBasicQuery<Security>(submodule, "SecurityQuery");
  ExportQueueSuite<Nexus::Queries::QueryVariant>(submodule, "QueryVariant");
  ExportQueueSuite<Quantity>(submodule, "Quantity");
  ExportQueueSuite<Money>(submodule, "Money");
  ExportQueueSuite<Security>(submodule, "Security");
  ExportQueueSuite<SecurityInfo>(submodule, "SecurityInfo");
}

void Nexus::Python::export_time_and_sale_accessor(module& module) {
  class_<TimeAndSaleAccessor>(module, "TimeAndSaleAccessor").
    def(init<Expression>()).
    def_property_readonly("timestamp", &TimeAndSaleAccessor::get_timestamp).
    def_property_readonly("price", &TimeAndSaleAccessor::get_price).
    def_property_readonly("size", &TimeAndSaleAccessor::get_size).
    def_property_readonly(
      "market_center", &TimeAndSaleAccessor::get_market_center).
    def_property_readonly("buyer_mpid", &TimeAndSaleAccessor::get_buyer_mpid).
    def_property_readonly("seller_mpid", &TimeAndSaleAccessor::get_seller_mpid);
}

void Nexus::Python::export_value(module& module) {
  ExportNativeValue<QuantityValue>(module, "QuantityValue");
  ExportNativeValue<MoneyValue>(module, "MoneyValue");
  ExportNativeValue<SecurityValue>(module, "SecurityValue");
  ExportNativeValue<SecurityInfoValue>(module, "SecurityInfoValue");
  ExportNativeValue<OrderImbalanceValue>(module, "OrderImbalanceValue");
  ExportNativeValue<BboQuoteValue>(module, "BboQuoteValue");
  ExportNativeValue<BookQuoteValue>(module, "BookQuoteValue");
  ExportNativeValue<TimeAndSaleValue>(module, "TimeAndSaleValue");
  ExportNativeValue<OrderFieldsValue>(module, "OrderFieldsValue");
  ExportNativeValue<OrderInfoValue>(module, "OrderInfoValue");
}
