#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/Array.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Copy.hpp>
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

void Nexus::Python::ExportAsxtFeeTable() {
  {
    scope outer = class_<AsxtFeeTable>("AsxtFeeTable", init<>())
      .def("__copy__", &MakeCopy<AsxtFeeTable>)
      .def("__deepcopy__", &MakeDeepCopy<AsxtFeeTable>)
      .def_readwrite("spire_fee", &AsxtFeeTable::m_spireFee)
      .add_property("clearing_rate_table",
        MakeArray(&AsxtFeeTable::m_clearingRateTable))
      .add_property("trade_rate", make_getter(&AsxtFeeTable::m_tradeRate,
        return_value_policy<return_by_value>()), make_setter(
        &AsxtFeeTable::m_tradeRate, return_value_policy<return_by_value>()))
      .add_property("gst_rate", make_getter(&AsxtFeeTable::m_gstRate,
        return_value_policy<return_by_value>()), make_setter(
        &AsxtFeeTable::m_gstRate, return_value_policy<return_by_value>()))
      .def_readwrite("trade_fee_cap", &AsxtFeeTable::m_tradeFeeCap);
    enum_<AsxtFeeTable::PriceClass>("PriceClass")
      .value("NONE", AsxtFeeTable::PriceClass::NONE)
      .value("TIER_ONE", AsxtFeeTable::PriceClass::TIER_ONE)
      .value("TIER_TWO", AsxtFeeTable::PriceClass::TIER_TWO)
      .value("TIER_THREE", AsxtFeeTable::PriceClass::TIER_THREE);
  }
  def("parse_asx_fee_table", &ParseAsxFeeTable);
  def("lookup_clearing_fee", &LookupClearingFee);
  def("lookup_price_class", &LookupPriceClass);
  def("calculate_fee", static_cast<ExecutionReport (*)(const AsxtFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportChicFeeTable() {
  {
    scope outer = class_<ChicFeeTable>("ChicFeeTable", init<>())
      .def("__copy__", &MakeCopy<ChicFeeTable>)
      .def("__deepcopy__", &MakeDeepCopy<ChicFeeTable>)
      .add_property("security_table",
      MakeArray(&ChicFeeTable::m_securityTable));
    enum_<ChicFeeTable::Classification>("Classification")
      .value("NONE", ChicFeeTable::Classification::NONE)
      .value("INTERLISTED", ChicFeeTable::Classification::INTERLISTED)
      .value("NON_INTERLISTED", ChicFeeTable::Classification::NON_INTERLISTED)
      .value("ETF", ChicFeeTable::Classification::ETF)
      .value("SUB_DOLLAR", ChicFeeTable::Classification::SUB_DOLLAR)
      .value("SUB_DIME", ChicFeeTable::Classification::SUB_DIME);
    enum_<ChicFeeTable::Index>("Index")
      .value("NONE", ChicFeeTable::Index::NONE)
      .value("ACTIVE", ChicFeeTable::Index::ACTIVE)
      .value("PASSIVE", ChicFeeTable::Index::PASSIVE)
      .value("HIDDEN_ACTIVE", ChicFeeTable::Index::HIDDEN_ACTIVE)
      .value("HIDDEN_PASSIVE", ChicFeeTable::Index::HIDDEN_PASSIVE);
  }
  def("parse_chic_fee_table", &ParseChicFeeTable);
  def("lookup_fee", static_cast<Money (*)(const ChicFeeTable&,
    ChicFeeTable::Index, ChicFeeTable::Classification)>(&LookupFee));
  def("calculate_fee", static_cast<Money (*)(const ChicFeeTable&,
    const OrderFields&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportConsolidatedTmxFeeTable() {
  {
    scope outer = class_<ConsolidatedTmxFeeTable>(
      "ConsolidatedTmxFeeTable", init<>())
      .def("__copy__", &MakeCopy<ConsolidatedTmxFeeTable>)
      .def("__deepcopy__", &MakeDeepCopy<ConsolidatedTmxFeeTable>)
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
    class_<ConsolidatedTmxFeeTable::State, noncopyable>("State", init<>())
      .def_readonly("per_order_charges",
        &ConsolidatedTmxFeeTable::State::m_perOrderCharges)
      .def_readonly("fill_count",
        &ConsolidatedTmxFeeTable::State::m_fillCount);
  }
  def("parse_consolidated_tmx_fee_table", &ParseConsolidatedTmxFeeTable);
  def("calculate_fee", static_cast<ExecutionReport (*)(
    const ConsolidatedTmxFeeTable&, ConsolidatedTmxFeeTable::State&,
    const Order&, const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportFeeHandling() {
  ExportAsxtFeeTable();
  ExportChicFeeTable();
  ExportConsolidatedTmxFeeTable();
  ExportLiquidityFlag();
  ExportPureFeeTable();
}

void Nexus::Python::ExportLiquidityFlag() {
  enum_<LiquidityFlag>("LiquidityFlag")
    .value("NONE", LiquidityFlag::NONE)
    .value("ACTIVE", LiquidityFlag::ACTIVE)
    .value("PASSIVE", LiquidityFlag::PASSIVE);
}

void Nexus::Python::ExportPureFeeTable() {
  class_<PureFeeTable>("PureFeeTable")
    .def("__copy__", &MakeCopy<PureFeeTable>)
    .def("__deepcopy__", &MakeDeepCopy<PureFeeTable>);
  def("parse_pure_fee_table", &ParsePureFeeTable);
  def("calculate_fee", static_cast<Money (*)(const PureFeeTable&,
    const Security&, const ExecutionReport&)>(&CalculateFee));
}
