#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/Beam.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/FeeHandling/AsxTradeMatchFeeTable.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandling/Cse2FeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_asx_trade_match_fee_table(module& module) {
  auto fee_table =
    class_<AsxTradeMatchFeeTable>(module, "AsxTradeMatchFeeTable").
      def(init()).
      def(init<const AsxTradeMatchFeeTable&>()).
      def_readwrite("spire_fee", &AsxTradeMatchFeeTable::m_spire_fee).
      def_readwrite("clearing_rate_table",
        &AsxTradeMatchFeeTable::m_clearing_rate_table).
      def_readwrite("trade_rate", &AsxTradeMatchFeeTable::m_trade_rate).
      def_readwrite("gst_rate", &AsxTradeMatchFeeTable::m_gst_rate).
      def_readwrite("trade_fee_cap", &AsxTradeMatchFeeTable::m_trade_fee_cap);
  enum_<AsxTradeMatchFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", AsxTradeMatchFeeTable::PriceClass::NONE).
    value("TIER_ONE", AsxTradeMatchFeeTable::PriceClass::TIER_ONE).
    value("TIER_TWO", AsxTradeMatchFeeTable::PriceClass::TIER_TWO).
    value("TIER_THREE", AsxTradeMatchFeeTable::PriceClass::TIER_THREE);
  enum_<AsxTradeMatchFeeTable::OrderTypeClass>(fee_table, "OrderTypeClass").
    value("NONE", AsxTradeMatchFeeTable::OrderTypeClass::NONE).
    value("REGULAR", AsxTradeMatchFeeTable::OrderTypeClass::REGULAR).
    value("PEGGED", AsxTradeMatchFeeTable::OrderTypeClass::PEGGED);
  module.def("parse_asx_trade_match_fee_table",
    static_cast<AsxTradeMatchFeeTable (*)(const YAML::Node&)>(
      &parse_asx_trade_match_fee_table));
  module.def("lookup_clearing_fee",
    static_cast<boost::rational<int> (*)(
      const AsxTradeMatchFeeTable&, AsxTradeMatchFeeTable::PriceClass,
      AsxTradeMatchFeeTable::OrderTypeClass)>(&lookup_clearing_fee));
  module.def("lookup_price_class",
    static_cast<AsxTradeMatchFeeTable::PriceClass (*)(const ExecutionReport&)>(
      &lookup_price_class));
  module.def("lookup_order_type_class",
    static_cast<AsxTradeMatchFeeTable::OrderTypeClass (*)(const OrderFields&)>(
      &lookup_order_type_class));
  module.def("calculate_clearing_fee",
    static_cast<Money (*)(const AsxTradeMatchFeeTable&, const OrderFields&,
      const ExecutionReport&)>(&calculate_clearing_fee));
  module.def("calculate_execution_fee", static_cast<Money (*)(
    const AsxTradeMatchFeeTable&, const ExecutionReport&)>(
      &calculate_execution_fee));
  module.def("calculate_fee", static_cast<ExecutionReport (*)(
    const AsxTradeMatchFeeTable&, const OrderFields&, const ExecutionReport&)>(
      &calculate_fee));
}

void Nexus::Python::export_chic_fee_table(module& module) {
  auto fee_table = class_<ChicFeeTable>(module, "ChicFeeTable").
    def(init()).
    def(init<const ChicFeeTable&>()).
    def_readwrite("security_table", &ChicFeeTable::m_security_table).
    def_readwrite("interlisted", &ChicFeeTable::m_interlisted).
    def_readwrite("etfs", &ChicFeeTable::m_etfs);
  enum_<ChicFeeTable::Classification>(fee_table, "Classification").
    value("NONE", ChicFeeTable::Classification::NONE).
    value("INTERLISTED", ChicFeeTable::Classification::INTERLISTED).
    value("NON_INTERLISTED", ChicFeeTable::Classification::NON_INTERLISTED).
    value("ETF", ChicFeeTable::Classification::ETF).
    value("SUBDOLLAR", ChicFeeTable::Classification::SUBDOLLAR).
    value("SUBDIME", ChicFeeTable::Classification::SUBDIME);
  enum_<ChicFeeTable::Index>(fee_table, "Index").
    value("NONE", ChicFeeTable::Index::NONE).
    value("ACTIVE", ChicFeeTable::Index::ACTIVE).
    value("PASSIVE", ChicFeeTable::Index::PASSIVE).
    value("HIDDEN_ACTIVE", ChicFeeTable::Index::HIDDEN_ACTIVE).
    value("HIDDEN_PASSIVE", ChicFeeTable::Index::HIDDEN_PASSIVE);
  module.def("parse_chic_fee_table",
    static_cast<ChicFeeTable (*)(const YAML::Node&,
      std::unordered_set<Security>, std::unordered_set<Security>)>(
        &parse_chic_fee_table));
  module.def("lookup_fee",
    static_cast<Money (*)(const ChicFeeTable&, ChicFeeTable::Index,
      ChicFeeTable::Classification)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const ChicFeeTable&, const OrderFields&,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_consolidated_tmx_fee_table(module& module) {
  auto table =
    class_<ConsolidatedTmxFeeTable>(module, "ConsolidatedTmxFeeTable").
      def(init()).
      def(init<const ConsolidatedTmxFeeTable&>()).
      def_readwrite("spire_fee", &ConsolidatedTmxFeeTable::m_spire_fee).
      def_readwrite("iiroc_fee", &ConsolidatedTmxFeeTable::m_iiroc_fee).
      def_readwrite("cds_fee", &ConsolidatedTmxFeeTable::m_cds_fee).
      def_readwrite("cds_cap", &ConsolidatedTmxFeeTable::m_cds_cap).
      def_readwrite("clearing_fee", &ConsolidatedTmxFeeTable::m_clearing_fee).
      def_readwrite("per_order_fee", &ConsolidatedTmxFeeTable::m_per_order_fee).
      def_readwrite("per_order_cap", &ConsolidatedTmxFeeTable::m_per_order_cap).
      def_readwrite("chic_fee_table",
        &ConsolidatedTmxFeeTable::m_chic_fee_table).
      def_readwrite("cse_fee_table",
        &ConsolidatedTmxFeeTable::m_cse_fee_table).
      def_readwrite("cse2_fee_table",
        &ConsolidatedTmxFeeTable::m_cse2_fee_table).
      def_readwrite("lynx_fee_table",
        &ConsolidatedTmxFeeTable::m_lynx_fee_table).
      def_readwrite("matn_fee_table",
        &ConsolidatedTmxFeeTable::m_matn_fee_table).
      def_readwrite("neoe_fee_table",
        &ConsolidatedTmxFeeTable::m_neoe_fee_table).
      def_readwrite("nex_fee_table",
        &ConsolidatedTmxFeeTable::m_nex_fee_table).
      def_readwrite("omga_fee_table",
        &ConsolidatedTmxFeeTable::m_omga_fee_table).
      def_readwrite("pure_fee_table",
        &ConsolidatedTmxFeeTable::m_pure_fee_table).
      def_readwrite("tsx_fee_table",
        &ConsolidatedTmxFeeTable::m_tsx_fee_table).
      def_readwrite("tsxv_fee_table",
        &ConsolidatedTmxFeeTable::m_tsxv_fee_table).
      def_readwrite("xats_fee_table",
        &ConsolidatedTmxFeeTable::m_xats_fee_table).
      def_readwrite("xcx2_fee_table",
        &ConsolidatedTmxFeeTable::m_xcx2_fee_table).
      def_readwrite("etfs", &ConsolidatedTmxFeeTable::m_etfs).
      def_readwrite("interlisted", &ConsolidatedTmxFeeTable::m_interlisted).
      def_readwrite("nex_listed", &ConsolidatedTmxFeeTable::m_nex_listed);
  class_<ConsolidatedTmxFeeTable::State>(table, "State").
    def(init()).
    def_readonly("per_order_charges",
      &ConsolidatedTmxFeeTable::State::m_per_order_charges).
    def_readonly("fill_count",
      &ConsolidatedTmxFeeTable::State::m_fill_count);
  module.def("parse_tmx_interlisted_securities",
    static_cast<std::unordered_set<Security> (*)(const std::string&,
      const VenueDatabase&)>(&parse_tmx_interlisted_securities));
  module.def("parse_tmx_etf_securities",
    static_cast<std::unordered_set<Security> (*)(const std::string&,
      const VenueDatabase&)>(&parse_tmx_etf_securities));
  module.def("parse_nex_listed_securities",
    static_cast<std::unordered_set<Security> (*)(const std::string&,
      const VenueDatabase&)>(&parse_nex_listed_securities));
  module.def("parse_consolidated_tmx_fee_table",
    static_cast<ConsolidatedTmxFeeTable (*)(const YAML::Node&,
      const VenueDatabase&)>(&parse_consolidated_tmx_fee_table));
  module.def("calculate_fee",
    static_cast<ExecutionReport (*)(const ConsolidatedTmxFeeTable&,
      ConsolidatedTmxFeeTable::State&, const Order&, const ExecutionReport&)>(
        &calculate_fee));
}

void Nexus::Python::export_cse_fee_table(module& module) {
  auto fee_table = class_<CseFeeTable>(module, "CseFeeTable").
    def(init()).
    def(init<const CseFeeTable&>()).
    def_readwrite("fee_table", &CseFeeTable::m_fee_table);
  enum_<CseFeeTable::Section>(fee_table, "Section").
    value("NONE", CseFeeTable::Section::NONE).
    value("DEFAULT", CseFeeTable::Section::DEFAULT).
    value("SUBDOLLAR", CseFeeTable::Section::SUBDOLLAR).
    value("DARK", CseFeeTable::Section::DARK);
  module.def("parse_cse_fee_table",
    static_cast<CseFeeTable (*)(const YAML::Node&)>(&parse_cse_fee_table));
  module.def("lookup_cse_fee_table_section",
    static_cast<CseFeeTable::Section (*)(const ExecutionReport&)>(
      &lookup_cse_fee_table_section));
  module.def("lookup_cse_liquidity_flag",
    static_cast<LiquidityFlag (*)(const ExecutionReport&)>(
      &lookup_cse_liquidity_flag));
  module.def("lookup_fee",
    static_cast<Money (*)(const CseFeeTable&, LiquidityFlag,
      CseFeeTable::Section)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const CseFeeTable&, const ExecutionReport&)>(
      &calculate_fee));
}

void Nexus::Python::export_cse2_fee_table(module& module) {
  auto fee_table = class_<Cse2FeeTable>(module, "Cse2FeeTable").
    def(init()).
    def(init<const Cse2FeeTable&>()).
    def_readwrite("regular_table", &Cse2FeeTable::m_regular_table).
    def_readwrite("dark_table", &Cse2FeeTable::m_dark_table).
    def_readwrite("debentures_or_notes_table",
      &Cse2FeeTable::m_debentures_or_notes_table).
    def_readwrite("cse_listed_government_bonds_table",
      &Cse2FeeTable::m_cse_listed_government_bonds_table).
    def_readwrite("oddlot_table", &Cse2FeeTable::m_oddlot_table);
  enum_<Cse2FeeTable::Section>(fee_table, "Section").
    value("REGULAR", Cse2FeeTable::Section::REGULAR).
    value("DARK", Cse2FeeTable::Section::DARK).
    value("DEBENTURES_OR_NOTES",
      Cse2FeeTable::Section::DEBENTURES_OR_NOTES).
    value("CSE_LISTED_GOVERNMENT_BONDS",
      Cse2FeeTable::Section::CSE_LISTED_GOVERNMENT_BONDS).
    value("ODDLOT", Cse2FeeTable::Section::ODDLOT);
  enum_<Cse2FeeTable::PriceClass>(fee_table, "PriceClass").
    value("DEFAULT", Cse2FeeTable::PriceClass::DEFAULT).
    value("SUBDOLLAR", Cse2FeeTable::PriceClass::SUBDOLLAR);
  enum_<Cse2FeeTable::ListingMarket>(fee_table, "ListingMarket").
    value("CSE", Cse2FeeTable::ListingMarket::CSE).
    value("TSX_TSXV", Cse2FeeTable::ListingMarket::TSX_TSXV);
  module.def("parse_cse2_fee_table",
    static_cast<Cse2FeeTable (*)(const YAML::Node&)>(&parse_cse2_fee_table));
  module.def("lookup_cse2_fee_table_section",
    static_cast<Cse2FeeTable::Section (*)(const OrderFields&,
      const ExecutionReport&)>(&lookup_cse2_fee_table_section));
  module.def("lookup_cse2_price_class",
    static_cast<Cse2FeeTable::PriceClass (*)(const ExecutionReport&)>(
      &lookup_cse2_price_class));
  module.def("lookup_cse2_liquidity_flag",
    static_cast<LiquidityFlag (*)(const ExecutionReport&)>(
      &lookup_cse2_liquidity_flag));
  module.def("lookup_cse2_listing_market",
    static_cast<Cse2FeeTable::ListingMarket (*)(const ExecutionReport&)>(
      &lookup_cse2_listing_market));
  module.def("lookup_regular_fee",
    static_cast<Money (*)(const Cse2FeeTable&, LiquidityFlag,
      Cse2FeeTable::PriceClass)>(&lookup_regular_fee));
  module.def("calculate_regular_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const ExecutionReport&)>(
      &calculate_regular_fee));
  module.def("lookup_dark_fee",
    static_cast<Money (*)(const Cse2FeeTable&, LiquidityFlag,
      Cse2FeeTable::PriceClass)>(&lookup_dark_fee));
  module.def("calculate_dark_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const ExecutionReport&)>(
      &calculate_dark_fee));
  module.def("lookup_debentures_or_notes_fee",
    static_cast<Money (*)(const Cse2FeeTable&, LiquidityFlag,
      Cse2FeeTable::ListingMarket)>(&lookup_debentures_or_notes_fee));
  module.def("calculate_debentures_or_notes_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const ExecutionReport&)>(
      &calculate_debentures_or_notes_fee));
  module.def("lookup_cse_listed_government_bonds_fee",
    static_cast<Money (*)(const Cse2FeeTable&, LiquidityFlag)>(
      &lookup_cse_listed_government_bonds_fee));
  module.def("calculate_cse_listed_government_bonds_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const ExecutionReport&)>(
      &calculate_cse_listed_government_bonds_fee));
  module.def("lookup_oddlot_fee",
    static_cast<Money (*)(const Cse2FeeTable&, LiquidityFlag,
      Cse2FeeTable::PriceClass)>(&lookup_oddlot_fee));
  module.def("calculate_oddlot_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const ExecutionReport&)>(
      &calculate_oddlot_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const Cse2FeeTable&, const OrderFields&,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_fee_handling(module& module) {
  export_asx_trade_match_fee_table(module);
  export_chic_fee_table(module);
  export_consolidated_tmx_fee_table(module);
  export_cse_fee_table(module);
  export_cse2_fee_table(module);
  export_liquidity_flag(module);
  export_lynx_fee_table(module);
  export_matn_fee_table(module);
  export_neoe_fee_table(module);
  export_nex_fee_table(module);
  export_omga_fee_table(module);
  export_parse_fee_table(module);
  export_pure_fee_table(module);
  export_tsx_fee_table(module);
  export_xats_fee_table(module);
  export_xcx2_fee_table(module);
}

void Nexus::Python::export_liquidity_flag(module& module) {
  enum_<LiquidityFlag>(module, "LiquidityFlag")
    .value("NONE", LiquidityFlag::NONE)
    .value("ACTIVE", LiquidityFlag::ACTIVE)
    .value("PASSIVE", LiquidityFlag::PASSIVE);
}

void Nexus::Python::export_lynx_fee_table(module& module) {
  auto fee_table = class_<LynxFeeTable>(module, "LynxFeeTable").
    def(init()).
    def(init<const LynxFeeTable&>()).
    def_readwrite("fee_table", &LynxFeeTable::m_fee_table).
    def_readwrite("interlisted", &LynxFeeTable::m_interlisted).
    def_readwrite("etfs", &LynxFeeTable::m_etfs);
  enum_<LynxFeeTable::Classification>(fee_table, "Classification").
    value("NONE", LynxFeeTable::Classification::NONE).
    value("DEFAULT", LynxFeeTable::Classification::DEFAULT).
    value("INTERLISTED", LynxFeeTable::Classification::INTERLISTED).
    value("ETF", LynxFeeTable::Classification::ETF).
    value("SUBDOLLAR", LynxFeeTable::Classification::SUBDOLLAR).
    value("MIDPOINT", LynxFeeTable::Classification::MIDPOINT);
  module.def("parse_lynx_fee_table",
    static_cast<LynxFeeTable (*)(const YAML::Node&,
      std::unordered_set<Security>, std::unordered_set<Security>)>(
        &parse_lynx_fee_table));
  module.def("is_lynx_midpoint_order",
    static_cast<bool (*)(const OrderFields&)>(&is_lynx_midpoint_order));
  module.def("lookup_fee",
    static_cast<Money (*)(const LynxFeeTable&, LiquidityFlag,
      LynxFeeTable::Classification)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const LynxFeeTable&, const OrderFields&,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_matn_fee_table(module& module) {
  auto fee_table = class_<MatnFeeTable>(module, "MatnFeeTable").
    def(init()).
    def(init<const MatnFeeTable&>()).
    def_readwrite("general_fee_table", &MatnFeeTable::m_general_fee_table).
    def_readwrite("alternative_fee_table",
      &MatnFeeTable::m_alternative_fee_table);
  enum_<MatnFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", MatnFeeTable::PriceClass::NONE).
    value("DEFAULT", MatnFeeTable::PriceClass::DEFAULT).
    value("SUBFIVE_DOLLAR", MatnFeeTable::PriceClass::SUBFIVE_DOLLAR).
    value("SUBDOLLAR", MatnFeeTable::PriceClass::SUBDOLLAR);
  enum_<MatnFeeTable::GeneralIndex>(fee_table, "GeneralIndex").
    value("NONE", MatnFeeTable::GeneralIndex::NONE).
    value("FEE", MatnFeeTable::GeneralIndex::FEE).
    value("MAX_CHARGE", MatnFeeTable::GeneralIndex::MAX_CHARGE);
  enum_<MatnFeeTable::Category>(fee_table, "Category").
    value("NONE", MatnFeeTable::Category::NONE).
    value("ETF", MatnFeeTable::Category::ETF).
    value("ODD_LOT", MatnFeeTable::Category::ODD_LOT);
  enum_<MatnFeeTable::Classification>(fee_table, "Classification").
    value("NONE", MatnFeeTable::Classification::NONE).
    value("DEFAULT", MatnFeeTable::Classification::DEFAULT).
    value("ETF", MatnFeeTable::Classification::ETF);
  module.def("parse_matn_fee_table",
    static_cast<MatnFeeTable (*)(const YAML::Node&)>(&parse_matn_fee_table));
  module.def("lookup_fee",
    static_cast<Money (*)(const MatnFeeTable&, MatnFeeTable::GeneralIndex,
      MatnFeeTable::PriceClass)>(&lookup_fee));
  module.def("lookup_fee",
    static_cast<Money (*)(const MatnFeeTable&, LiquidityFlag,
      MatnFeeTable::Category)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const MatnFeeTable&, MatnFeeTable::Classification,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_neoe_fee_table(module& module) {
  auto fee_table = class_<NeoeFeeTable>(module, "NeoeFeeTable").
    def(init()).
    def(init<const NeoeFeeTable&>()).
    def_readwrite("general_fee_table", &NeoeFeeTable::m_general_fee_table).
    def_readwrite("interlisted_fee_table",
      &NeoeFeeTable::m_interlisted_fee_table).
    def_readwrite("etf_table_fee", &NeoeFeeTable::m_etf_table_fee).
    def_readwrite("neo_book_fee_table", &NeoeFeeTable::m_neo_book_fee_table);
  enum_<NeoeFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", NeoeFeeTable::PriceClass::NONE).
    value("SUBDOLLAR", NeoeFeeTable::PriceClass::SUBDOLLAR).
    value("DEFAULT", NeoeFeeTable::PriceClass::DEFAULT);
  enum_<NeoeFeeTable::Classification>(fee_table, "Classification").
    value("GENERAL", NeoeFeeTable::Classification::GENERAL).
    value("INTERLISTED", NeoeFeeTable::Classification::INTERLISTED).
    value("ETF", NeoeFeeTable::Classification::ETF);
  module.def("parse_neoe_fee_table",
    static_cast<NeoeFeeTable (*)(const YAML::Node&)>(&parse_neoe_fee_table));
  module.def("is_neo_book_order",
    static_cast<bool (*)(const OrderFields&)>(&is_neo_book_order));
  module.def("lookup_general_fee",
    static_cast<Money (*)(const NeoeFeeTable&, LiquidityFlag,
      NeoeFeeTable::PriceClass)>(&lookup_general_fee));
  module.def("lookup_interlisted_fee",
    static_cast<Money (*)(const NeoeFeeTable&, LiquidityFlag,
      NeoeFeeTable::PriceClass)>(&lookup_interlisted_fee));
  module.def("lookup_etf_fee",
    static_cast<Money (*)(const NeoeFeeTable&, LiquidityFlag,
      NeoeFeeTable::PriceClass)>(&lookup_etf_fee));
  module.def("lookup_neo_book_fee",
    static_cast<Money (*)(const NeoeFeeTable&, LiquidityFlag,
      NeoeFeeTable::PriceClass)>(&lookup_neo_book_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const NeoeFeeTable&, NeoeFeeTable::Classification,
      const OrderFields&, const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_nex_fee_table(module& module) {
  class_<NexFeeTable>(module, "NexFeeTable").
    def(init()).
    def(init<const NexFeeTable&>()).
    def_readwrite("fee", &NexFeeTable::m_fee);
  module.def("parse_nex_fee_table",
    static_cast<NexFeeTable (*)(const YAML::Node&)>(&parse_nex_fee_table));
  module.def("calculate_fee",
    static_cast<Money (*)(const NexFeeTable&, const ExecutionReport&)>(
      &calculate_fee));
}

void Nexus::Python::export_omga_fee_table(module& module) {
  auto fee_table = class_<OmgaFeeTable>(module, "OmgaFeeTable").
    def(init()).
    def(init<const OmgaFeeTable&>()).
    def_readwrite("fee_table", &OmgaFeeTable::m_fee_table);
  enum_<OmgaFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", OmgaFeeTable::PriceClass::NONE).
    value("DEFAULT", OmgaFeeTable::PriceClass::DEFAULT).
    value("SUBDOLLAR", OmgaFeeTable::PriceClass::SUBDOLLAR);
  enum_<OmgaFeeTable::Type>(fee_table, "Type").
    value("NONE", OmgaFeeTable::Type::NONE).
    value("PASSIVE", OmgaFeeTable::Type::PASSIVE).
    value("ACTIVE", OmgaFeeTable::Type::ACTIVE).
    value("HIDDEN_PASSIVE", OmgaFeeTable::Type::HIDDEN_PASSIVE).
    value("HIDDEN_ACTIVE", OmgaFeeTable::Type::HIDDEN_ACTIVE).
    value("ETF_PASSIVE", OmgaFeeTable::Type::ETF_PASSIVE).
    value("ETF_ACTIVE", OmgaFeeTable::Type::ETF_ACTIVE).
    value("ODD_LOT", OmgaFeeTable::Type::ODD_LOT);
  module.def("parse_omga_fee_table",
    static_cast<OmgaFeeTable (*)(const YAML::Node&)>(&parse_omga_fee_table));
  module.def("lookup_fee",
    static_cast<Money (*)(const OmgaFeeTable&, OmgaFeeTable::Type,
      OmgaFeeTable::PriceClass)>(&lookup_fee));
  module.def("is_omga_hidden_liquidity_provider", static_cast<
    bool (*)(const OrderFields&)>(&is_omga_hidden_liquidity_provider));
  module.def("calculate_fee",
    static_cast<Money (*)(const OmgaFeeTable&, bool, const OrderFields&,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_parse_fee_table(module& module) {
  module.def("parse_fee_table", [] (const YAML::Node& node) {
    auto table = std::unordered_map<std::string, Money>();
    parse_fee_table<Money>(node, Store(table));
    return table;
  });
  module.def("parse_fee_table",
    [] (const YAML::Node& node, const std::string& name) {
      auto table = std::unordered_map<std::string, Money>();
      parse_fee_table<Money>(node, name, Store(table));
      return table;
    });
}

void Nexus::Python::export_pure_fee_table(module& module) {
  auto fee_table = class_<PureFeeTable>(module, "PureFeeTable").
    def(init()).
    def(init<const PureFeeTable&>()).
    def_readwrite("fee_table", &PureFeeTable::m_fee_table);
  enum_<PureFeeTable::Section>(fee_table, "Section").
    value("NONE", PureFeeTable::Section::NONE).
    value("DEFAULT", PureFeeTable::Section::DEFAULT).
    value("INTERLISTED", PureFeeTable::Section::INTERLISTED).
    value("ETF", PureFeeTable::Section::ETF);
  enum_<PureFeeTable::Row>(fee_table, "Row").
    value("NONE", PureFeeTable::Row::NONE).
    value("SUBDOLLAR", PureFeeTable::Row::SUBDOLLAR).
    value("DEFAULT", PureFeeTable::Row::DEFAULT).
    value("DARK_SUBDOLLAR", PureFeeTable::Row::DARK_SUBDOLLAR).
    value("DARK", PureFeeTable::Row::DARK);
  module.def("parse_pure_fee_table",
    static_cast<PureFeeTable (*)(const YAML::Node&)>(&parse_pure_fee_table));
  module.def("lookup_pure_row",
    static_cast<PureFeeTable::Row (*)(const ExecutionReport&)>(
      &lookup_pure_row));
  module.def("lookup_pure_liquidity_flag",
    static_cast<LiquidityFlag (*)(const ExecutionReport&)>(
      &lookup_pure_liquidity_flag));
  module.def("lookup_fee",
    static_cast<Money (*)(const PureFeeTable&, PureFeeTable::Section,
      PureFeeTable::Row, LiquidityFlag)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const PureFeeTable&, PureFeeTable::Section,
      const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_tsx_fee_table(module& module) {
  auto fee_table = class_<TsxFeeTable>(module, "TsxFeeTable").
    def(init()).
    def(init<const TsxFeeTable&>()).
    def_readwrite("continuous_fee_table", &TsxFeeTable::m_continuous_fee_table).
    def_readwrite("auction_fee_table", &TsxFeeTable::m_auction_fee_table).
    def_readwrite("odd_lot_fee_list", &TsxFeeTable::m_odd_lot_fee_list);
  enum_<TsxFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", TsxFeeTable::PriceClass::NONE).
    value("SUBDIME", TsxFeeTable::PriceClass::SUBDIME).
    value("SUBDOLLAR", TsxFeeTable::PriceClass::SUBDOLLAR).
    value("DEFAULT", TsxFeeTable::PriceClass::DEFAULT).
    value("DEFAULT_INTERLISTED", TsxFeeTable::PriceClass::DEFAULT_INTERLISTED).
    value("DEFAULT_ETF", TsxFeeTable::PriceClass::DEFAULT_ETF);
  enum_<TsxFeeTable::Type>(fee_table, "Type").
    value("NONE", TsxFeeTable::Type::NONE).
    value("ACTIVE", TsxFeeTable::Type::ACTIVE).
    value("PASSIVE", TsxFeeTable::Type::PASSIVE).
    value("HIDDEN_ACTIVE", TsxFeeTable::Type::HIDDEN_ACTIVE).
    value("HIDDEN_PASSIVE", TsxFeeTable::Type::HIDDEN_PASSIVE);
  enum_<TsxFeeTable::AuctionIndex>(fee_table, "AuctionIndex").
    value("NONE", TsxFeeTable::AuctionIndex::NONE).
    value("FEE", TsxFeeTable::AuctionIndex::FEE).
    value("MAX_CHARGE", TsxFeeTable::AuctionIndex::MAX_CHARGE);
  enum_<TsxFeeTable::AuctionType>(fee_table, "AuctionType").
    value("NONE", TsxFeeTable::AuctionType::NONE).
    value("OPEN", TsxFeeTable::AuctionType::OPEN).
    value("CLOSE", TsxFeeTable::AuctionType::CLOSE);
  enum_<TsxFeeTable::Classification>(fee_table, "Classification").
    value("NONE", TsxFeeTable::Classification::NONE).
    value("DEFAULT", TsxFeeTable::Classification::DEFAULT).
    value("ETF", TsxFeeTable::Classification::ETF).
    value("INTERLISTED", TsxFeeTable::Classification::INTERLISTED);
  module.def("parse_tsx_fee_table",
    static_cast<TsxFeeTable (*)(const YAML::Node&)>(&parse_tsx_fee_table));
  module.def("lookup_continuous_fee",
    static_cast<Money (*)(const TsxFeeTable&, TsxFeeTable::PriceClass,
      TsxFeeTable::Type)>(&lookup_continuous_fee));
  module.def("lookup_auction_fee",
    static_cast<Money (*)(const TsxFeeTable&, TsxFeeTable::AuctionIndex,
      TsxFeeTable::AuctionType)>(&lookup_auction_fee));
  module.def("lookup_odd_lot_fee",
    static_cast<Money (*)(const TsxFeeTable&, TsxFeeTable::PriceClass)>(
      &lookup_odd_lot_fee));
  module.def("is_tsx_hidden_order",
    static_cast<bool (*)(const OrderFields&)>(&is_tsx_hidden_order));
  module.def("calculate_fee",
    static_cast<Money (*)(const TsxFeeTable&, TsxFeeTable::Classification,
      const OrderFields&, const ExecutionReport&)>(&calculate_fee));
}

void Nexus::Python::export_xats_fee_table(module& module) {
  auto fee_table = class_<XatsFeeTable>(module, "XatsFeeTable").
    def(init()).
    def(init<const XatsFeeTable&>()).
    def_readwrite("general_fee_table", &XatsFeeTable::m_general_fee_table).
    def_readwrite("etf_fee_table", &XatsFeeTable::m_etf_fee_table).
    def_readwrite("intraspread_dark_to_dark_subdollar_max_fee",
      &XatsFeeTable::m_intraspread_dark_to_dark_subdollar_max_fee).
    def_readwrite("intraspread_dark_to_dark_max_fee",
      &XatsFeeTable::m_intraspread_dark_to_dark_max_fee);
  enum_<XatsFeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", XatsFeeTable::PriceClass::NONE).
    value("SUBHALF_DOLLAR", XatsFeeTable::PriceClass::SUBHALF_DOLLAR).
    value("SUBDOLLAR", XatsFeeTable::PriceClass::SUBDOLLAR).
    value("SUBFIVE_DOLLAR", XatsFeeTable::PriceClass::SUBFIVE_DOLLAR).
    value("DEFAULT", XatsFeeTable::PriceClass::DEFAULT);
  enum_<XatsFeeTable::Type>(fee_table, "Type").
    value("NONE", XatsFeeTable::Type::NONE).
    value("ACTIVE", XatsFeeTable::Type::ACTIVE).
    value("PASSIVE", XatsFeeTable::Type::PASSIVE).
    value("OPEN_AUCTION", XatsFeeTable::Type::OPEN_AUCTION).
    value("SDL_ACTIVE", XatsFeeTable::Type::SDL_ACTIVE).
    value("SDL_PASSIVE", XatsFeeTable::Type::SDL_PASSIVE).
    value("INTRASPREAD_ACTIVE", XatsFeeTable::Type::INTRASPREAD_ACTIVE).
    value("INTRASPREAD_PASSIVE", XatsFeeTable::Type::INTRASPREAD_PASSIVE);
  module.def("parse_xats_fee_table",
    static_cast<XatsFeeTable (*)(const YAML::Node&)>(&parse_xats_fee_table));
  module.def("lookup_general_fee",
    static_cast<Money (*)(const XatsFeeTable&, XatsFeeTable::Type,
      XatsFeeTable::PriceClass)>(&lookup_general_fee));
  module.def("lookup_etf_fee",
    static_cast<Money (*)(const XatsFeeTable&, XatsFeeTable::Type,
      XatsFeeTable::PriceClass)>(&lookup_etf_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const XatsFeeTable&, bool, const ExecutionReport&)>(
      &calculate_fee));
}

void Nexus::Python::export_xcx2_fee_table(module& module) {
  auto fee_table = class_<Xcx2FeeTable>(module, "Xcx2FeeTable").
    def(init()).
    def(init<const Xcx2FeeTable&>()).
    def_readwrite("default_table", &Xcx2FeeTable::m_default_table).
    def_readwrite("tsx_table", &Xcx2FeeTable::m_tsx_table).
    def_readwrite("large_trade_size", &Xcx2FeeTable::m_large_trade_size).
    def_readwrite("etfs", &Xcx2FeeTable::m_etfs);
  enum_<Xcx2FeeTable::PriceClass>(fee_table, "PriceClass").
    value("NONE", Xcx2FeeTable::PriceClass::NONE).
    value("DEFAULT", Xcx2FeeTable::PriceClass::DEFAULT).
    value("ETF", Xcx2FeeTable::PriceClass::ETF).
    value("SUB_FIVE_DOLLAR", Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR).
    value("SUBDOLLAR", Xcx2FeeTable::PriceClass::SUBDOLLAR).
    value("SUBDIME", Xcx2FeeTable::PriceClass::SUBDIME);
  enum_<Xcx2FeeTable::Type>(fee_table, "Type").
    value("NONE", Xcx2FeeTable::Type::NONE).
    value("ACTIVE", Xcx2FeeTable::Type::ACTIVE).
    value("PASSIVE", Xcx2FeeTable::Type::PASSIVE).
    value("LARGE_ACTIVE", Xcx2FeeTable::Type::LARGE_ACTIVE).
    value("LARGE_PASSIVE", Xcx2FeeTable::Type::LARGE_PASSIVE).
    value("HIDDEN_ACTIVE", Xcx2FeeTable::Type::HIDDEN_ACTIVE).
    value("HIDDEN_PASSIVE", Xcx2FeeTable::Type::HIDDEN_PASSIVE).
    value("ODD_LOT", Xcx2FeeTable::Type::ODD_LOT);
  module.def("parse_xcx2_fee_table",
    static_cast<Xcx2FeeTable (*)(const YAML::Node&,
      std::unordered_set<Security>)>(&parse_xcx2_fee_table));
  module.def("lookup_fee",
    static_cast<Money (*)(const Xcx2FeeTable&, const OrderFields&,
      Xcx2FeeTable::Type, Xcx2FeeTable::PriceClass)>(&lookup_fee));
  module.def("calculate_fee",
    static_cast<Money (*)(const Xcx2FeeTable&, const OrderFields&,
      const ExecutionReport&)>(&calculate_fee));
}
