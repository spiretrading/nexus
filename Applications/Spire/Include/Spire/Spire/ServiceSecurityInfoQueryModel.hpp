#ifndef SPIRE_SERVICE_SECURITY_INFO_QUERY_MODEL_HPP
#define SPIRE_SERVICE_SECURITY_INFO_QUERY_MODEL_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by submitting queries for securities to a market 
   * data client.
   */
  class ServiceSecurityInfoQueryModel : public QueryModel<Nexus::SecurityInfo> {
    public:

      /**
       * Constructs a ServiceSecurityInfoQueryModel.
       * @param markets The database of markets used for parsing securities.
       * @param market_data_client The MarketDataClient to submit queries to.
       */
      ServiceSecurityInfoQueryModel(Nexus::MarketDatabase m_markets,
        Nexus::MarketDataService::MarketDataClientBox market_data_client);

      boost::optional<Nexus::SecurityInfo> parse(const QString& query) override;

      QtPromise<std::vector<Nexus::SecurityInfo>>
        submit(const QString& query) override;

    private:
      Nexus::MarketDatabase m_markets;
      Nexus::MarketDataService::MarketDataClientBox m_market_data_client;
      Beam::SynchronizedUnorderedMap<Nexus::Security, Nexus::SecurityInfo>
        m_info;
  };
}

#endif
