#ifndef NEXUS_RISK_PORTFOLIO_TYPES_HPP
#define NEXUS_RISK_PORTFOLIO_TYPES_HPP
#include <functional>
#include <ostream>
#include <boost/functional/hash.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/KeyValuePair.hpp>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Used as a key into an account's Inventory. */
  struct RiskPortfolioKey {

    /** The portfolio's account. */
    Beam::DirectoryEntry m_account;

    /** The Ticker being indexed. */
    Ticker m_ticker;

    bool operator ==(const RiskPortfolioKey&) const = default;
  };

  /** The type of Portfolio used. */
  using RiskPortfolio = Portfolio<TrueAverageBookkeeper>;

  /** Stores an Inventory update. */
  using RiskInventoryEntry = Beam::KeyValuePair<RiskPortfolioKey, Inventory>;

  /** The Publisher used for portfolio events. */
  using RiskPortfolioUpdatePublisher =
    Beam::Publisher<Beam::KeyValuePair<RiskPortfolioKey, Inventory>>;

  inline std::ostream& operator <<(
      std::ostream& out, const RiskPortfolioKey& key) {
    return out << '(' << key.m_account << ' ' << key.m_ticker << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::RiskPortfolioKey> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::RiskPortfolioKey& value,
        unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("ticker", value.m_ticker);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::RiskPortfolioKey> {
    size_t operator()(const Nexus::RiskPortfolioKey& value) const {
      auto seed = std::size_t(0);
      boost::hash_combine(
        seed, std::hash<Beam::DirectoryEntry>()(value.m_account));
      boost::hash_combine(seed, std::hash<Nexus::Ticker>()(value.m_ticker));
      return seed;
    }
  };
}

#endif
