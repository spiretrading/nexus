#ifndef SPIRE_SERVICE_SECURITY_QUERY_MODEL_HPP
#define SPIRE_SERVICE_SECURITY_QUERY_MODEL_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by submitting queries for securities to a market 
   * data client.
   */
  class ServiceSecurityQueryModel : public QueryModel<Nexus::SecurityInfo> {
    public:

      /**
       * Constructs a ServiceSecurityQueryModel.
       * @param market_data_client The MarketDataClient to submit queries to.
       */
      explicit ServiceSecurityQueryModel(
        Nexus::MarketDataClient market_data_client);

      boost::optional<Nexus::SecurityInfo> parse(const QString& query) override;
      QtPromise<std::vector<Nexus::SecurityInfo>>
        submit(const QString& query) override;

    private:
      Nexus::MarketDataClient m_market_data_client;
      Beam::SynchronizedUnorderedMap<Nexus::Security, Nexus::SecurityInfo>
        m_info;
  };
}

#endif
