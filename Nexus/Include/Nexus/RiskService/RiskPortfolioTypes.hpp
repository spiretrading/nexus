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
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Money.hpp"

namespace Nexus::RiskService {

  /** Used as a key into an account's Inventory. */
  struct RiskPortfolioKey {

    /** The portfolio's account. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The Security being indexed. */
    Security m_security;

    bool operator ==(const RiskPortfolioKey&) const = default;
  };

  /** The type of Portfolio used. */
  using RiskPortfolio =
    Accounting::Portfolio<Accounting::TrueAverageBookkeeper>;

  /** Stores an Inventory update. */
  using RiskInventoryEntry =
    Beam::KeyValuePair<RiskPortfolioKey, Accounting::Inventory>;

  /** The Publisher used for portfolio events. */
  using RiskPortfolioUpdatePublisher = Beam::Publisher<
    Beam::KeyValuePair<RiskPortfolioKey, Accounting::Inventory>>;

  inline std::ostream& operator <<(
      std::ostream& out, const RiskPortfolioKey& key) {
    return out << '(' << key.m_account << ' ' << key.m_security << ')';
  }

  inline std::size_t hash_value(const RiskPortfolioKey& value) {
    auto seed = std::size_t(0);
    boost::hash_combine(seed, value.m_account);
    boost::hash_combine(seed, value.m_security);
    return seed;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::RiskPortfolioKey> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, Nexus::RiskService::RiskPortfolioKey& value,
        unsigned int version) const {
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("security", value.m_security);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::RiskService::RiskPortfolioKey> {
    size_t operator()(const Nexus::RiskService::RiskPortfolioKey& value) const {
      return Nexus::RiskService::hash_value(value);
    }
  };
}

#endif
