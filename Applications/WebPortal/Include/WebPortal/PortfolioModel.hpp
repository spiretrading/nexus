#ifndef WEB_PORTAL_PORTFOLIO_MODEL_HPP
#define WEB_PORTAL_PORTFOLIO_MODEL_HPP
#include <memory>
#include <unordered_map>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /** Publishes updates for positions held by trading accounts. */
  class PortfolioModel {
    public:

      /** Represents a single position. */
      struct Entry {

        /** The account holding the position. */
        Beam::DirectoryEntry m_account;

        /** The Entry's Inventory. */
        Inventory m_inventory;

        /** The position's unrealized profit and loss. */
        boost::optional<Money> m_unrealized_profit_and_loss;

        /**
         * Constructs an Entry.
         * @param account The account holding the position.
         * @param security The position's Security.
         * @param currency The position's currency.
         */
        Entry(Beam::DirectoryEntry account, Security security,
          CurrencyId currency);

        /**
         * Tests if two Entry's are equal.
         * @param rhs The right hand side of the comparison.
         * @return true iff the two Entry's represent the same
         *         account, security and currency.
         */
        bool operator ==(const Entry& rhs) const;
      };

      /**
       * Constructs a PortfolioModel.
       * @param clients The Clients used to query for positions.
       */
      explicit PortfolioModel(Clients clients);

      ~PortfolioModel();

      /** Returns the Publisher updating the position Entries. */
      const Beam::Publisher<Entry>& get_publisher() const;

      void close();

    private:
      Clients m_clients;
      std::unordered_map<RiskPortfolioKey, std::shared_ptr<Entry>> m_entries;
      std::unordered_map<Security, std::vector<std::shared_ptr<Entry>>>
        m_security_to_entries;
      Beam::QueueWriterPublisher<Entry> m_publisher;
      std::unordered_map<Security, SecurityValuation> m_valuations;
      Beam::RoutineTaskQueue m_tasks;

      PortfolioModel(const PortfolioModel&) = delete;
      PortfolioModel& operator=(const PortfolioModel&) = delete;
      void on_risk_portfolio_inventory_update(
        const RiskInventoryEntry& inventory);
      void on_bbo_quote(const Security& security, SecurityValuation& valuation,
        const BboQuote& quote);
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::PortfolioModel::Entry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::PortfolioModel::Entry& entry,
        unsigned int version) const {
      shuttle.shuttle("account", entry.m_account);
      shuttle.shuttle("inventory", entry.m_inventory);
      shuttle.shuttle(
        "unrealized_profit_and_loss", entry.m_unrealized_profit_and_loss);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::PortfolioModel::Entry> {
    size_t operator()(const Nexus::PortfolioModel::Entry& value) const;
  };
}

#endif
