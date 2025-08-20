#include "Nexus/Python/FeeHandling.hpp"
#include <pybind11/pybind11.h>
#include "Nexus/FeeHandling/AsxTradeMatchFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"

using namespace Nexus;
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
  module.def(
    "parse_asx_trade_match_fee_table", &parse_asx_trade_match_fee_table);
  module.def("lookup_clearing_fee", &lookup_clearing_fee);
  module.def("lookup_price_class", &lookup_price_class);
  module.def("lookup_order_type_class", &lookup_order_type_class);
  module.def("calculate_clearing_fee", &calculate_clearing_fee);
  module.def("calculate_execution_fee", &calculate_execution_fee);
  module.def("calculate_fee", &calculate_fee);
}

void Nexus::Python::export_chic_fee_table(module& module) {
}

void Nexus::Python::export_consolidated_tmx_fee_table(module& module) {
}

void Nexus::Python::export_cse_fee_table(module& module) {
}

void Nexus::Python::export_cse2_fee_table(module& module) {
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
}

void Nexus::Python::export_matn_fee_table(module& module) {
}

void Nexus::Python::export_neoe_fee_table(module& module) {
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
