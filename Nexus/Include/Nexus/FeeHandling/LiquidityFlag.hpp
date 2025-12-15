#ifndef NEXUS_LIQUIDITY_FLAG_HPP
#define NEXUS_LIQUIDITY_FLAG_HPP
#include <ostream>

namespace Nexus {

  /** Enumerates liquidity flags. */
  enum class LiquidityFlag {

    /** No liquidity flag specified. */
    NONE = -1,

    /** The trade removed liquidity. */
    ACTIVE = 0,

    /** The trade provided liquidity. */
    PASSIVE
  };

  /** The number of liquidity flags enumerated. */
  static constexpr auto LIQUIDITY_FLAG_COUNT = std::size_t(2);

  inline std::ostream& operator <<(std::ostream& out, LiquidityFlag flag) {
    if(flag == LiquidityFlag::NONE) {
      return out;
    } else if(flag == LiquidityFlag::ACTIVE) {
      return out << 'A';
    } else if(flag == LiquidityFlag::PASSIVE) {
      return out << 'P';
    } else {
      return out << '?';
    }
  }
}

#endif
