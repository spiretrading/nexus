#ifndef WEB_PORTAL_PORTFOLIO_MODEL_HPP
#define WEB_PORTAL_PORTFOLIO_MODEL_HPP
#include <memory>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "WebPortal/WebPortal.hpp"

namespace Nexus::WebPortal {

  /** Publishes updates for positions held by trading accounts. */
  class PortfolioModel : private boost::noncopyable {
    public:

      /** Represents a single position. */
      struct Entry {

        /** The account holding the position. */
        Beam::ServiceLocator::DirectoryEntry m_account;

        /** The Entry's Inventory. */
        RiskService::RiskInventory m_inventory;

        /** The position's unrealized profit and loss. */
        boost::optional<Money> m_unrealizedProfitAndLoss;

        /**
         * Constructs an Entry.
         * @param account The account holding the position.
         * @param security The position's Security.
         * @param currency The position's currency.
         */
        Entry(Beam::ServiceLocator::DirectoryEntry account, Security security,
          CurrencyId currency);

        /**
         * Tests if two Entry's are equal.
         * @param rhs The right hand side of the comparison.
         * @return <code>true</code> iff the two Entry's represent the same
         *         account, security and currency.
         */
        bool operator ==(const Entry& rhs) const;
      };

      /**
       * Constructs a PortfolioModel.
       * @param serviceClients The ServiceClients used to query for positions.
       */
      explicit PortfolioModel(Beam::Ref<VirtualServiceClients> serviceClients);

      ~PortfolioModel();

      /** Returns the Publisher updating the position Entries. */
      const Beam::Publisher<Entry>& GetPublisher() const;

      void Open();

      void Close();

    private:
      VirtualServiceClients* m_serviceClients;
      std::unordered_map<RiskService::RiskPortfolioKey, std::shared_ptr<Entry>>
        m_entries;
      std::unordered_map<Security, std::vector<std::shared_ptr<Entry>>>
        m_securityToEntries;
      Beam::QueueWriterPublisher<Entry> m_publisher;
      std::unordered_map<Security, Accounting::SecurityValuation> m_valuations;
      Beam::RoutineTaskQueue m_tasks;

      void OnRiskPortfolioInventoryUpdate(
        const RiskService::RiskInventoryEntry& inventory);
      void OnBboQuote(const Security& security,
        Accounting::SecurityValuation& valuation, const BboQuote& quote);
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::WebPortal::PortfolioModel::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::WebPortal::PortfolioModel::Entry& entry,
        unsigned int version) {
      shuttle.Shuttle("account", entry.m_account);
      shuttle.Shuttle("inventory", entry.m_inventory);
      shuttle.Shuttle("unrealized_profit_and_loss",
        entry.m_unrealizedProfitAndLoss);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::WebPortal::PortfolioModel::Entry> {
    size_t operator()(
      const Nexus::WebPortal::PortfolioModel::Entry& value) const;
  };
}

#endif
