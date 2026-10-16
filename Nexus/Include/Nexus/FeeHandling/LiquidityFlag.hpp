#ifndef NEXUS_LIQUIDITY_FLAG_HPP
#define NEXUS_LIQUIDITY_FLAG_HPP
#include <format>
#include <ostream>
#include <string_view>

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
}

template<>
class std::formatter<Nexus::LiquidityFlag> :
    public std::formatter<std::string_view> {
  public:
    auto format(Nexus::LiquidityFlag flag, auto& context) const {
      auto text = [&] {
        switch(flag) {
          case Nexus::LiquidityFlag::NONE:
            return std::string_view();
          case Nexus::LiquidityFlag::ACTIVE:
            return std::string_view("A");
          case Nexus::LiquidityFlag::PASSIVE:
            return std::string_view("P");
          default:
            return std::string_view("?");
        }
      }();
      return std::formatter<std::string_view>::format(text, context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(std::ostream& out, LiquidityFlag flag) {
    return out << std::format(out.getloc(), "{}", flag);
  }
}

#endif
