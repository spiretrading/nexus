#ifndef NEXUS_RISK_PORTFOLIO_SUBSCRIPTION_HPP
#define NEXUS_RISK_PORTFOLIO_SUBSCRIPTION_HPP
#include <Beam/Queues/Queue.hpp>
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /** Supplies a portfolio snapshot followed by its subsequent updates. */
  class RiskPortfolioSubscription {
    public:

      /** Subscribes to the portfolio published by an authenticated client. */
      explicit RiskPortfolioSubscription(
        const RiskPortfolioUpdatePublisher& publisher);
      ~RiskPortfolioSubscription();

      /** Returns the initial portfolio snapshot. */
      const std::vector<RiskInventoryEntry>& get_snapshot() const;

      /** Returns the next portfolio update, waiting until one is available. */
      RiskInventoryEntry pop();

      void close();

    private:
      std::shared_ptr<Beam::Queue<RiskInventoryEntry>> m_updates;
      std::vector<RiskInventoryEntry> m_snapshot;

      RiskPortfolioSubscription(const RiskPortfolioSubscription&) = delete;
      RiskPortfolioSubscription& operator =(
        const RiskPortfolioSubscription&) = delete;
  };
}

#endif
