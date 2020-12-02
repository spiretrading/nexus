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

void Nexus::Python::ExportDataType(module& module) {
  ExportNativeDataType<QuantityType>(module, "QuantityType");
  ExportNativeDataType<MoneyType>(module, "MoneyType");
  ExportNativeDataType<SecurityType>(module, "SecurityType");
  ExportNativeDataType<OrderImbalanceType>(module, "OrderImbalanceType");
  ExportNativeDataType<BboQuoteType>(module, "BboQuoteType");
  ExportNativeDataType<BookQuoteType>(module, "BookQuoteType");
  ExportNativeDataType<MarketQuoteType>(module, "MarketQuoteType");
  ExportNativeDataType<TimeAndSaleType>(module, "TimeAndSaleType");
  ExportNativeDataType<OrderFieldsType>(module, "OrderFieldsType");
  ExportNativeDataType<OrderInfoType>(module, "OrderInfoType");
}

void Nexus::Python::ExportQueries(module& module) {
  auto submodule = module.def_submodule("queries");
  ExportDataType(submodule);
  ExportValue(submodule);
}

void Nexus::Python::ExportValue(module& module) {
  ExportNativeValue<QuantityValue>(module, "QuantityValue");
  ExportNativeValue<MoneyValue>(module, "MoneyValue");
  ExportNativeValue<SecurityValue>(module, "SecurityValue");
  ExportNativeValue<OrderImbalanceValue>(module, "OrderImbalanceValue");
  ExportNativeValue<BboQuoteValue>(module, "BboQuoteValue");
  ExportNativeValue<BookQuoteValue>(module, "BookQuoteValue");
  ExportNativeValue<MarketQuoteValue>(module, "MarketQuoteValue");
  ExportNativeValue<TimeAndSaleValue>(module, "TimeAndSaleValue");
  ExportNativeValue<OrderFieldsValue>(module, "OrderFieldsValue");
  ExportNativeValue<OrderInfoValue>(module, "OrderInfoValue");
}
