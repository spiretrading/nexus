#ifndef NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#define NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/Sequence.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
  class BacktesterMarketDataService : private boost::noncopyable {
    public:
      BacktesterMarketDataService(
        Beam::RefType<BacktesterEventHandler> eventHandler);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

    private:
      struct SecuritySequences {
        Beam::Queries::Sequence m_bboSequence;
        Beam::Queries::Sequence m_timeAndSaleSequence;
      };
      BacktesterEventHandler* m_eventHandler;
      std::unordered_map<Security, SecuritySequences> m_securitSequences;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
      Beam::RefType<BacktesterEventHandler> eventHandler)
      : m_eventHandler{eventHandler.Get()} {}

  inline void BacktesterMarketDataService::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
  }
}

#endif
