#ifndef CLIENTWEBPORTAL_PORTFOLIOMODEL_HPP
#define CLIENTWEBPORTAL_PORTFOLIOMODEL_HPP
#include <memory>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "ClientWebPortal/ClientWebPortal/ClientWebPortal.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"

namespace Nexus {
namespace ClientWebPortal {
  class PortfolioModel : private boost::noncopyable {
    public:
      struct Entry {
        Beam::ServiceLocator::DirectoryEntry m_account;

        Security m_security;

        CurrencyId m_currency;

        Quantity m_openQuantity;

        Money m_averagePrice;

        Money m_totalProfitAndLoss;

        Money m_unrealizedProfitAndLoss;

        Money m_realizedProfitAndLoss;

        Money m_fees;

        Money m_costBasis;

        Quantity m_volume;

        int m_trades;

        Entry(Beam::ServiceLocator::DirectoryEntry account, Security security,
          CurrencyId currency);

        bool operator ==(const Entry& rhs) const;
      };

      PortfolioModel(Beam::RefType<ApplicationServiceClients> serviceClients);

      ~PortfolioModel();

      void Open();

      void Close();

      const Beam::Publisher<Entry>& GetPublisher() const;

    private:
      ApplicationServiceClients* m_serviceClients;
      std::unordered_map<RiskService::RiskPortfolioKey, std::shared_ptr<Entry>>
        m_entries;
      std::unordered_map<Security, std::vector<std::shared_ptr<Entry>>>
        m_securityToEntries;
      Beam::MultiQueueWriter<Entry> m_publisher;
      Beam::RoutineTaskQueue m_tasks;

      void OnRiskPortfolioInventoryUpdate(
        const RiskService::RiskPortfolioInventoryEntry& inventory);
  };
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::ClientWebPortal::PortfolioModel::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::ClientWebPortal::PortfolioModel::Entry& entry,
        unsigned int version) {
      shuttle.Shuttle("account", entry.m_account);
      shuttle.Shuttle("security", entry.m_security);
      shuttle.Shuttle("currency", entry.m_currency);
      shuttle.Shuttle("open_quantity", entry.m_openQuantity);
      shuttle.Shuttle("average_price", entry.m_averagePrice);
      shuttle.Shuttle("total_profit_and_loss", entry.m_totalProfitAndLoss);
      shuttle.Shuttle("unrealized_profit_and_loss",
        entry.m_unrealizedProfitAndLoss);
      shuttle.Shuttle("realized_profit_and_loss",
        entry.m_realizedProfitAndLoss);
      shuttle.Shuttle("fees", entry.m_fees);
      shuttle.Shuttle("cost_basis", entry.m_costBasis);
      shuttle.Shuttle("volume", entry.m_volume);
      shuttle.Shuttle("trades", entry.m_trades);
    }
  };
}
}

namespace std {
  template <>
  struct hash<Nexus::ClientWebPortal::PortfolioModel::Entry> {
    size_t operator()(
      const Nexus::ClientWebPortal::PortfolioModel::Entry& value) const;
  };
};

#endif
