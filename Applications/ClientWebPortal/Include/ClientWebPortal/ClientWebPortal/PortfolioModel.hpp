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

  /*! \class PortfolioModel
      \brief Publishes updates for positions held by trading accounts.
   */
  class PortfolioModel : private boost::noncopyable {
    public:

      /*! \struct Entry
          \brief Represents a single position.
       */
      struct Entry {

        //! The account holding the position.
        Beam::ServiceLocator::DirectoryEntry m_account;

        //! The position's Security.
        Security m_security;

        //! The position's currency.
        CurrencyId m_currency;

        //! The position's quantity.
        Quantity m_openQuantity;

        //! The average price of the position.
        Money m_averagePrice;

        //! The position's total profit and loss.
        Money m_totalProfitAndLoss;

        //! The position's unrealized profit and loss.
        Money m_unrealizedProfitAndLoss;

        //! The position's realized profit and loss.
        Money m_realizedProfitAndLoss;

        //! The amount of fees paid for this entry.
        Money m_fees;

        //! The position's cost basis.
        Money m_costBasis;

        //! The quantity traded for this entry.
        Quantity m_volume;

        //! The number of trades made for this entry.
        int m_trades;

        //! Constructs an Entry.
        /*!
          \param account The account holding the position.
          \param security The position's Security.
          \param currency The position's currency.
        */
        Entry(Beam::ServiceLocator::DirectoryEntry account, Security security,
          CurrencyId currency);

        //! Tests if two Entry's are equal.
        /*!
          \param rhs The right hand side of the comparison.
          \return <code>true</code> iff the two Entry's represent the same
                  account, security and currency.
        */
        bool operator ==(const Entry& rhs) const;
      };

      //! Constructs a PortfolioModel.
      /*!
        \param serviceClients The ServiceClients used to query for positions.
      */
      PortfolioModel(Beam::RefType<ApplicationServiceClients> serviceClients);

      ~PortfolioModel();

      void Open();

      void Close();

      //! Returns the Publisher updating the position Entries.
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
