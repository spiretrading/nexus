#ifndef NEXUS_RISK_PORTFOLIO_TYPES_HPP
#define NEXUS_RISK_PORTFOLIO_TYPES_HPP
#include <functional>
#include <boost/functional/hash.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/KeyValuePair.hpp>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Used as a key into an account's Inventory. */
  struct RiskPortfolioKey {

    /** The portfolio's account. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The Security being indexed. */
    Security m_security;

    /** Constructs a RiskPortfolioKey. */
    RiskPortfolioKey() = default;

    /**
     * Constructs a RiskPortfolioKey.
     * @param account The portfolio's account.
     * @param security The Security being indexed.
     */
    RiskPortfolioKey(Beam::ServiceLocator::DirectoryEntry account,
      Security security);

    /** Tests for equality. */
    bool operator ==(const RiskPortfolioKey& key) const;

    /** Tests for equality. */
    bool operator !=(const RiskPortfolioKey& key) const;
  };

  /** The type used to represent a portfolio's position. */
  using RiskPosition = Accounting::Position<Security>;

  /** The type used to represent a portfolio's Inventory. */
  using RiskInventory = Accounting::Inventory<RiskPosition>;

  /** The type of Portfolio used. */
  using RiskPortfolio = Accounting::Portfolio<
    Accounting::TrueAverageBookkeeper<RiskInventory>>;

  /** The type of valuation used. */
  using RiskSecurityValuation = Accounting::SecurityValuation;

  /** Stores an Inventory update. */
  using RiskInventoryEntry =
    Beam::KeyValuePair<RiskPortfolioKey, RiskInventory>;

  /** The Publisher used for portfolio events. */
  using RiskPortfolioUpdatePublisher = Beam::Publisher<
    Beam::KeyValuePair<RiskPortfolioKey, RiskInventory>>;

  inline std::size_t hash_value(const RiskPortfolioKey& value) {
    auto seed = std::size_t(0);
    boost::hash_combine(seed, value.m_account);
    boost::hash_combine(seed, value.m_security);
    return seed;
  }

  inline RiskPortfolioKey::RiskPortfolioKey(
    Beam::ServiceLocator::DirectoryEntry account, Security security)
    : m_account(std::move(account)),
      m_security(std::move(security)) {}

  inline bool RiskPortfolioKey::operator ==(const RiskPortfolioKey& key) const {
    return m_account == key.m_account && m_security == key.m_security;
  }

  inline bool RiskPortfolioKey::operator !=(const RiskPortfolioKey& key) const {
    return !(*this == key);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::RiskPortfolioKey> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::RiskPortfolioKey& value,
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
};

#endif
