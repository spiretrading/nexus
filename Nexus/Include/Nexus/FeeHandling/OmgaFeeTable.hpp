#ifndef NEXUS_OMGA_FEE_TABLE_HPP
#define NEXUS_OMGA_FEE_TABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores the table of fees used by Omega on TSX. */
  struct OmgaFeeTable {

    /** Enumerates the types of price classes. */
    enum class PriceClass {

      /** Unknown. */
      NONE = -1,

      /** Price >= $1.00. */
      DEFAULT = 0,

      /** Price < $1.00. */
      SUBDOLLAR
    };

    /** The number of price classes enumerated. */
    static constexpr auto PRICE_CLASS_COUNT = std::size_t(2);

    /** Enumerates the types of trades. */
    enum class Type {

      /** Unknown. */
      NONE = -1,

      /** Passive. */
      PASSIVE = 0,

      /** Active. */
      ACTIVE,

      /** Hidden Passive. */
      HIDDEN_PASSIVE,

      /** Hidden Active. */
      HIDDEN_ACTIVE,

      /** ETF Passive. */
      ETF_PASSIVE,

      /** ETF Active. */
      ETF_ACTIVE,

      /** Odd Lot. */
      ODD_LOT
    };

    /** The number of trade types enumerated. */
    static constexpr auto TYPE_COUNT = std::size_t(7);

    /** The fee table. */
    std::array<std::array<Money, TYPE_COUNT>, PRICE_CLASS_COUNT> m_fee_table;
  };

  /**
   * Parses an OmgaFeeTable from a YAML configuration.
   * @param config The configuration to parse the OmgaFeeTable from.
   * @return The OmgaFeeTable represented by the <i>config</i>.
   */
  inline OmgaFeeTable parse_omga_fee_table(const YAML::Node& config) {
    auto table = OmgaFeeTable();
    parse_fee_table(config, "fee_table", Beam::out(table.m_fee_table));
    return table;
  }

  /**
   * Looks up a fee.
   * @param table The OmgaFeeTable used to lookup the fee.
   * @param type The trade's Type.
   * @param price_class The trade's PriceClass.
   * @return The fee corresponding to the specified <i>type</i> and
   *         <i>price_class</i>.
   */
  inline Money lookup_fee(const OmgaFeeTable& table, OmgaFeeTable::Type type,
      OmgaFeeTable::PriceClass price_class) {
    return table.m_fee_table[static_cast<int>(price_class)][
      static_cast<int>(type)];
  }

  /**
   * Tests if an OrderFields represents a hidden liquidity provider.
   * @param fields The OrderFields to test.
   * @return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
   *         provider.
   */
  inline bool is_omga_hidden_liquidity_provider(const OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      has_field(fields, Tag{18, "M"});
  }

  /**
   * Calculates the fee on a trade executed on OMGA.
   * @param table The OmgaFeeTable used to calculate the fee.
   * @param is_etf Whether the calculation is for an ETF.
   * @param order The Order the trade took place on.
   * @param report The ExecutionReport to calculate the fee for.
   * @return The fee calculated for the specified trade.
   */
  inline Money calculate_fee(const OmgaFeeTable& table, bool is_etf,
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return Money::ZERO;
    }
    auto price_class = [&] {
      if(report.m_last_price < Money::ONE) {
        return OmgaFeeTable::PriceClass::SUBDOLLAR;
      } else {
        return OmgaFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto type = [&] {
      if(report.m_last_quantity < 100) {
        return OmgaFeeTable::Type::ODD_LOT;
      } else if(is_omga_hidden_liquidity_provider(fields)) {
        if(report.m_liquidity_flag.size() == 1) {
          if(report.m_liquidity_flag[0] == 'A') {
            return OmgaFeeTable::Type::HIDDEN_ACTIVE;
          } else if(report.m_liquidity_flag[0] == 'P') {
            return OmgaFeeTable::Type::HIDDEN_PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              report.m_liquidity_flag << "\"\n";
            return OmgaFeeTable::Type::HIDDEN_PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return OmgaFeeTable::Type::HIDDEN_PASSIVE;
        }
      } else if(is_etf) {
        if(report.m_liquidity_flag.size() == 1) {
          if(report.m_liquidity_flag[0] == 'A') {
            return OmgaFeeTable::Type::ETF_ACTIVE;
          } else if(report.m_liquidity_flag[0] == 'P') {
            return OmgaFeeTable::Type::ETF_PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              report.m_liquidity_flag << "\"\n";
            return OmgaFeeTable::Type::ETF_PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return OmgaFeeTable::Type::ETF_PASSIVE;
        }
      } else {
        if(report.m_liquidity_flag.size() == 1) {
          if(report.m_liquidity_flag[0] == 'A') {
            return OmgaFeeTable::Type::ACTIVE;
          } else if(report.m_liquidity_flag[0] == 'P') {
            return OmgaFeeTable::Type::PASSIVE;
          } else {
            std::cout << "Unknown liquidity flag [OMGA]: \"" <<
              report.m_liquidity_flag << "\"\n";
            return OmgaFeeTable::Type::PASSIVE;
          }
        } else {
          std::cout << "Unknown liquidity flag [OMGA]: \"" <<
            report.m_liquidity_flag << "\"\n";
          return OmgaFeeTable::Type::PASSIVE;
        }
      }
    }();
    auto fee = lookup_fee(table, type, price_class);
    return report.m_last_quantity * fee;
  }
}

#endif
