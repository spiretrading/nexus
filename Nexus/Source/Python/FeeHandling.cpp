#include "Nexus/Python/FeeHandling.hpp"
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
}

void Nexus::Python::export_omga_fee_table(module& module) {
}

void Nexus::Python::export_parse_fee_table(module& module) {
}

void Nexus::Python::export_pure_fee_table(module& module) {
}

void Nexus::Python::export_tsx_fee_table(module& module) {
}

void Nexus::Python::export_xats_fee_table(module& module) {
}

void Nexus::Python::export_xcx2_fee_table(module& module) {
}
