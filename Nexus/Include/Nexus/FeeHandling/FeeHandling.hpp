#ifndef NEXUS_FEE_HANDLING_HPP
#define NEXUS_FEE_HANDLING_HPP
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"

namespace Nexus {
  struct AmexFeeTable;
  struct ArcaFeeTable;
  struct AsxtFeeTable;
  struct BatyFeeTable;
  struct BatsFeeTable;
  struct ChicFeeTable;
  struct ConsolidatedTmxFeeTable;
  struct ConsolidatedUsFeeTable;
  struct CseFeeTable;
  struct Cse2FeeTable;
  struct EdgaFeeTable;
  struct EdgxFeeTable;
  struct HkexFeeTable;
  struct JpxFeeTable;
  struct LynxFeeTable;
  enum class LiquidityFlag;
  struct MatnFeeTable;
  struct NeoeFeeTable;
  struct NexFeeTable;
  struct NsdqFeeTable;
  struct NyseFeeTable;
  struct OmgaFeeTable;
  struct PureFeeTable;
  struct TsxFeeTable;
  struct XatsFeeTable;
  struct Xcx2FeeTable;

  /**
   * Parses the set of symbols from a YAML config.
   * @param config The config to parse.
   * @param marketDatabase The MarketDatabase used to parse the symbols.
   * @return The set of symbols.
   */
  inline std::unordered_set<Security> ParseSecuritySet(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    auto securities = std::unordered_set<Security>();
    for(auto& item : config) {
      auto security = ParseSecurity(item.as<std::string>(), marketDatabase);
      securities.insert(security);
    }
    return securities;
  }
}

#endif
