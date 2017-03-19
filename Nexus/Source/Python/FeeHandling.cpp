#include "Nexus/Python/FeeHandling.hpp"
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
  class_<AsxtFeeTable>("AsxtFeeTable", init<>())
    .def("__copy__", &MakeCopy<AsxtFeeTable>)
    .def("__deepcopy__", &MakeDeepCopy<AsxtFeeTable>)
    .def_readwrite("spire_fee", &AsxtFeeTable::m_spireFee)
    .def_readwrite("clearing_rate", &AsxtFeeTable::m_clearingRate)
    .def_readwrite("trade_rate", &AsxtFeeTable::m_tradeRate)
    .def_readwrite("gst_rate", &AsxtFeeTable::m_gstRate)
    .def_readwrite("trade_fee_cap", &AsxtFeeTable::m_tradeFeeCap);
  def("parse_asx_fee_table", &ParseAsxFeeTable);
  def("calculate_fee", static_cast<ExecutionReport (*)(const AsxtFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportChicFeeTable() {
  {
    scope outer = class_<ChicFeeTable>("ChicFeeTable", init<>())
      .def("__copy__", &MakeCopy<ChicFeeTable>)
      .def("__deepcopy__", &MakeDeepCopy<ChicFeeTable>)
      .def_readwrite("fee_table", &ChicFeeTable::m_feeTable);
    enum_<ChicFeeTable::Category>("Category")
      .value("NONE", ChicFeeTable::Category::NONE)
      .value("DEFAULT", ChicFeeTable::Category::DEFAULT)
      .value("SUB_DOLLAR", ChicFeeTable::Category::SUB_DOLLAR)
      .value("INTERLISTED", ChicFeeTable::Category::INTERLISTED)
      .value("ETF", ChicFeeTable::Category::ETF);
    enum_<ChicFeeTable::Type>("Type")
      .value("NONE", ChicFeeTable::Type::NONE)
      .value("ACTIVE", ChicFeeTable::Type::ACTIVE)
      .value("PASSIVE", ChicFeeTable::Type::PASSIVE)
      .value("HIDDEN", ChicFeeTable::Type::HIDDEN);
  }
  def("parse_chic_fee_table", &ParseChicFeeTable);
  def("lookup_fee", static_cast<Money (*)(const ChicFeeTable&,
    ChicFeeTable::Type, ChicFeeTable::Category)>(&LookupFee));
  def("is_chic_hidden_liquidity_provider", &IsChicHiddenLiquidityProvider);
  def("calculate_fee", static_cast<Money (*)(const ChicFeeTable&, bool, bool,
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
