#include "Nexus/Python/Queries.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Queries.hpp>
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::Queries;
using namespace boost;
using namespace boost::python;
using namespace std;

void Nexus::Python::ExportDataType() {
  ExportNativeDataType<QuantityType>("QuantityType");
  ExportNativeDataType<MoneyType>("MoneyType");
  ExportNativeDataType<SecurityType>("SecurityType");
  ExportNativeDataType<OrderImbalanceType>("OrderImbalanceType");
  ExportNativeDataType<BboQuoteType>("BboQuoteType");
  ExportNativeDataType<BookQuoteType>("BookQuoteType");
  ExportNativeDataType<MarketQuoteType>("MarketQuoteType");
  ExportNativeDataType<TimeAndSaleType>("TimeAndSaleType");
  ExportNativeDataType<OrderFieldsType>("OrderFieldsType");
  ExportNativeDataType<OrderInfoType>("OrderInfoType");
}

void Nexus::Python::ExportQueries() {
  string nestedName = extract<string>(scope().attr("__name__") + ".queries");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("queries") = nestedModule;
  scope parent = nestedModule;
  ExportDataType();
  ExportValue();
}

void Nexus::Python::ExportValue() {
  ExportNativeValue<QuantityValue>("QuantityValue");
  ExportNativeValue<MoneyValue>("MoneyValue");
  ExportNativeValue<SecurityValue>("SecurityValue");
  ExportNativeValue<OrderImbalanceValue>("OrderImbalanceValue");
  ExportNativeValue<BboQuoteValue>("BboQuoteValue");
  ExportNativeValue<BookQuoteValue>("BookQuoteValue");
  ExportNativeValue<MarketQuoteValue>("MarketQuoteValue");
  ExportNativeValue<TimeAndSaleValue>("TimeAndSaleValue");
  ExportNativeValue<OrderFieldsValue>("OrderFieldsValue");
  ExportNativeValue<OrderInfoValue>("OrderInfoValue");
}
