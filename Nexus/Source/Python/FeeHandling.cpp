#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/Beam.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/FeeHandling/AmexFeeTable.hpp"
#include "Nexus/FeeHandling/ArcaFeeTable.hpp"
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/HkexFeeTable.hpp"
#include "Nexus/FeeHandling/JpxFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::ExportAmexFeeTable(pybind11::module& module) {
  auto outer = class_<AmexFeeTable>(module, "AmexFeeTable")
    .def_readwrite("fee_table", &AmexFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &AmexFeeTable::m_subdollarTable);
  enum_<AmexFeeTable::Type>(outer, "Type")
    .value("ACTIVE", AmexFeeTable::Type::ACTIVE)
    .value("PASSIVE", AmexFeeTable::Type::PASSIVE)
    .value("HIDDEN_ACTIVE", AmexFeeTable::Type::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", AmexFeeTable::Type::HIDDEN_PASSIVE)
    .value("AT_THE_OPEN", AmexFeeTable::Type::AT_THE_OPEN)
    .value("AT_THE_CLOSE", AmexFeeTable::Type::AT_THE_CLOSE)
    .value("ROUTED", AmexFeeTable::Type::ROUTED);
  module.def("parse_amex_fee_table", &ParseAmexFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(
    const AmexFeeTable&, AmexFeeTable::Type)>(&LookupFee));
  module.def("lookup_subdollar_fee", static_cast<rational<int> (*)(
    const AmexFeeTable&, AmexFeeTable::Type)>(&LookupSubdollarFee));
  module.def("is_amex_hidden_liquidity_provider",
    &IsAmexHiddenLiquidityProvider);
  module.def("calculate_fee", static_cast<Money (*)(const AmexFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportArcaFeeTable(pybind11::module& module) {
  auto outer = class_<ArcaFeeTable>(module, "ArcaFeeTable")
    .def_readwrite("fee_table", &ArcaFeeTable::m_feeTable)
    .def_readwrite("subdollar_table", &ArcaFeeTable::m_subdollarTable)
    .def_readwrite("routed_fee", &ArcaFeeTable::m_routedFee)
    .def_readwrite("auction_fee", &ArcaFeeTable::m_auctionFee);
  enum_<ArcaFeeTable::Category>(outer, "Category")
    .value("DEFAULT", ArcaFeeTable::Category::DEFAULT)
    .value("ROUTED", ArcaFeeTable::Category::ROUTED)
    .value("AUCTION", ArcaFeeTable::Category::AUCTION);
  enum_<ArcaFeeTable::Tape>(outer, "Tape")
    .value("A", ArcaFeeTable::Tape::A)
    .value("B", ArcaFeeTable::Tape::B)
    .value("C", ArcaFeeTable::Tape::C);
  enum_<ArcaFeeTable::Type>(outer, "Type")
    .value("ACTIVE", ArcaFeeTable::Type::ACTIVE)
    .value("PASSIVE", ArcaFeeTable::Type::PASSIVE)
    .value("HIDDEN_ACTIVE", ArcaFeeTable::Type::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", ArcaFeeTable::Type::HIDDEN_PASSIVE)
    .value("AT_THE_OPEN", ArcaFeeTable::Type::AT_THE_OPEN)
    .value("AT_THE_CLOSE", ArcaFeeTable::Type::AT_THE_CLOSE);
  enum_<ArcaFeeTable::SubdollarType>(outer, "SubdollarType")
    .value("ROUTED", ArcaFeeTable::SubdollarType::ROUTED)
    .value("AUCTION", ArcaFeeTable::SubdollarType::AUCTION)
    .value("ACTIVE", ArcaFeeTable::SubdollarType::ACTIVE)
    .value("PASSIVE", ArcaFeeTable::SubdollarType::PASSIVE);
  module.def("parse_arca_fee_table", &ParseArcaFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const ArcaFeeTable&,
    ArcaFeeTable::Tape, ArcaFeeTable::Type)>(&LookupFee));
  module.def("lookup_fee", static_cast<rational<int> (*)(const ArcaFeeTable&,
    ArcaFeeTable::SubdollarType)>(&LookupFee));
  module.def("is_arca_hidden_liquidity_provider",
    &IsArcaHiddenLiquidityProvider);
  module.def("calculate_fee", static_cast<Money (*)(const ArcaFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportAsxtFeeTable(pybind11::module& module) {
  auto outer = class_<AsxtFeeTable>(module, "AsxtFeeTable")
    .def(init())
    .def(init<const AsxtFeeTable&>())
    .def_readwrite("spire_fee", &AsxtFeeTable::m_spireFee)
    .def_readwrite("clearing_rate_table", &AsxtFeeTable::m_clearingRateTable)
    .def_readwrite("trade_rate", &AsxtFeeTable::m_tradeRate)
    .def_readwrite("gst_rate", &AsxtFeeTable::m_gstRate)
    .def_readwrite("trade_fee_cap", &AsxtFeeTable::m_tradeFeeCap);
  enum_<AsxtFeeTable::PriceClass>(outer, "PriceClass")
    .value("NONE", AsxtFeeTable::PriceClass::NONE)
    .value("TIER_ONE", AsxtFeeTable::PriceClass::TIER_ONE)
    .value("TIER_TWO", AsxtFeeTable::PriceClass::TIER_TWO)
    .value("TIER_THREE", AsxtFeeTable::PriceClass::TIER_THREE);
  enum_<AsxtFeeTable::OrderTypeClass>(outer, "OrderTypeClass")
    .value("NONE", AsxtFeeTable::OrderTypeClass::NONE)
    .value("REGULAR", AsxtFeeTable::OrderTypeClass::REGULAR)
    .value("PEGGED", AsxtFeeTable::OrderTypeClass::PEGGED);
  module.def("parse_asx_fee_table", &ParseAsxFeeTable);
  module.def("lookup_clearing_fee", &LookupClearingFee);
  module.def("lookup_price_class", &LookupPriceClass);
  module.def("lookup_order_type_class", &LookupOrderTypeClass);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const AsxtFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportChicFeeTable(pybind11::module& module) {
  auto outer = class_<ChicFeeTable>(module, "ChicFeeTable")
    .def(init())
    .def(init<const ChicFeeTable&>())
    .def_readwrite("security_table", &ChicFeeTable::m_securityTable);
  enum_<ChicFeeTable::Classification>(outer, "Classification")
    .value("NONE", ChicFeeTable::Classification::NONE)
    .value("INTERLISTED", ChicFeeTable::Classification::INTERLISTED)
    .value("NON_INTERLISTED", ChicFeeTable::Classification::NON_INTERLISTED)
    .value("ETF", ChicFeeTable::Classification::ETF)
    .value("SUBDOLLAR", ChicFeeTable::Classification::SUBDOLLAR)
    .value("SUBDIME", ChicFeeTable::Classification::SUBDIME);
  enum_<ChicFeeTable::Index>(outer, "Index")
    .value("NONE", ChicFeeTable::Index::NONE)
    .value("ACTIVE", ChicFeeTable::Index::ACTIVE)
    .value("PASSIVE", ChicFeeTable::Index::PASSIVE)
    .value("HIDDEN_ACTIVE", ChicFeeTable::Index::HIDDEN_ACTIVE)
    .value("HIDDEN_PASSIVE", ChicFeeTable::Index::HIDDEN_PASSIVE);
  module.def("parse_chic_fee_table", &ParseChicFeeTable);
  module.def("lookup_fee", static_cast<Money (*)(const ChicFeeTable&,
    ChicFeeTable::Index, ChicFeeTable::Classification)>(&LookupFee));
  module.def("calculate_fee", static_cast<Money (*)(const ChicFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportConsolidatedTmxFeeTable(pybind11::module& module) {
  auto outer = class_<ConsolidatedTmxFeeTable>(module,
      "ConsolidatedTmxFeeTable")
    .def(init())
    .def(init<const ConsolidatedTmxFeeTable&>())
    .def_readwrite("spire_fee", &ConsolidatedTmxFeeTable::m_spireFee)
    .def_readwrite("iiroc_fee", &ConsolidatedTmxFeeTable::m_iirocFee)
    .def_readwrite("cds_fee", &ConsolidatedTmxFeeTable::m_cdsFee)
    .def_readwrite("cds_cap", &ConsolidatedTmxFeeTable::m_cdsCap)
    .def_readwrite("per_order_fee", &ConsolidatedTmxFeeTable::m_perOrderFee)
    .def_readwrite("per_order_cap", &ConsolidatedTmxFeeTable::m_perOrderCap)
    .def_readwrite("chic_fee_table", &ConsolidatedTmxFeeTable::m_chicFeeTable)
    .def_readwrite("lynx_fee_table", &ConsolidatedTmxFeeTable::m_lynxFeeTable)
    .def_readwrite("matn_fee_table", &ConsolidatedTmxFeeTable::m_matnFeeTable)
    .def_readwrite("neoe_fee_table", &ConsolidatedTmxFeeTable::m_neoeFeeTable)
    .def_readwrite("omga_fee_table", &ConsolidatedTmxFeeTable::m_omgaFeeTable)
    .def_readwrite("pure_fee_table", &ConsolidatedTmxFeeTable::m_pureFeeTable)
    .def_readwrite("tsx_fee_table", &ConsolidatedTmxFeeTable::m_tsxFeeTable)
    .def_readwrite("tsx_venture_table",
      &ConsolidatedTmxFeeTable::m_tsxVentureTable)
    .def_readwrite("xats_fee_table", &ConsolidatedTmxFeeTable::m_xatsFeeTable)
    .def_readwrite("xcx2_fee_table", &ConsolidatedTmxFeeTable::m_xcx2FeeTable)
    .def_readwrite("etfs", &ConsolidatedTmxFeeTable::m_etfs)
    .def_readwrite("interlisted", &ConsolidatedTmxFeeTable::m_interlisted);
  class_<ConsolidatedTmxFeeTable::State>(outer, "State")
    .def(init())
    .def_readonly("per_order_charges",
      &ConsolidatedTmxFeeTable::State::m_perOrderCharges)
    .def_readonly("fill_count",
      &ConsolidatedTmxFeeTable::State::m_fillCount);
  module.def("parse_consolidated_tmx_fee_table", &ParseConsolidatedTmxFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const ConsolidatedTmxFeeTable&, ConsolidatedTmxFeeTable::State&,
    const Order&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportFeeHandling(pybind11::module& module) {
  ExportAmexFeeTable(module);
  ExportArcaFeeTable(module);
  ExportAsxtFeeTable(module);
  ExportChicFeeTable(module);
  ExportConsolidatedTmxFeeTable(module);
  ExportHkexFeeTable(module);
  ExportJpxFeeTable(module);
  ExportLiquidityFlag(module);
  ExportPureFeeTable(module);
}

void Nexus::Python::ExportHkexFeeTable(pybind11::module& module) {
  auto outer = class_<HkexFeeTable>(module, "HkexFeeTable")
    .def(init())
    .def(init<const HkexFeeTable&>())
    .def_readwrite("spire_fee", &HkexFeeTable::m_spireFee)
    .def_readwrite("stamp_tax", &HkexFeeTable::m_stampTax)
    .def_readwrite("levy", &HkexFeeTable::m_levy)
    .def_readwrite("trading_fee", &HkexFeeTable::m_tradingFee)
    .def_readwrite("brokerage_fee", &HkexFeeTable::m_brokerageFee)
    .def_readwrite("ccass_fee", &HkexFeeTable::m_ccassFee)
    .def_readwrite("minimum_ccass_fee", &HkexFeeTable::m_minimumCcassFee)
    .def_readwrite("stamp_applicability", &HkexFeeTable::m_stampApplicability);
  module.def("parse_hkex_stamp_securities", &ParseHkexStampSecurities);
  module.def("parse_hkex_fee_table", &ParseHkexFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const HkexFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportJpxFeeTable(pybind11::module& module) {
  auto outer = class_<JpxFeeTable>(module, "JpxFeeTable")
    .def(init())
    .def(init<const JpxFeeTable&>())
    .def_readwrite("spire_fee", &JpxFeeTable::m_spireFee)
    .def_readwrite("brokerage_fee", &JpxFeeTable::m_brokerageFee);
  module.def("parse_jpx_fee_table", &ParseJpxFeeTable);
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const JpxFeeTable&, const OrderFields&, const ExecutionReport&)>(
    &CalculateFee));
}

void Nexus::Python::ExportLiquidityFlag(pybind11::module& module) {
  enum_<LiquidityFlag>(module, "LiquidityFlag")
    .value("NONE", LiquidityFlag::NONE)
    .value("ACTIVE", LiquidityFlag::ACTIVE)
    .value("PASSIVE", LiquidityFlag::PASSIVE)
    .def("__str__", &lexical_cast<std::string, LiquidityFlag>);
}

void Nexus::Python::ExportPureFeeTable(pybind11::module& module) {
  class_<PureFeeTable>(module, "PureFeeTable")
    .def(init<const PureFeeTable&>());
  module.def("parse_pure_fee_table", &ParsePureFeeTable);
  module.def("calculate_fee", static_cast<Money (*)(const PureFeeTable&,
    const Security&, const ExecutionReport&)>(&CalculateFee));
}
