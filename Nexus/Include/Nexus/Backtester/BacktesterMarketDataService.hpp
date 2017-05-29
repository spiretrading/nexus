#ifndef NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#define NEXUS_BACKTESTERMARKETDATASERVICE_HPP
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/Sequence.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {
  class BacktesterMarketDataService : private boost::noncopyable {
    public:
      BacktesterMarketDataService(
        Beam::RefType<BacktesterEventHandler> eventHandler,
        Beam::RefType<MarketDataService::VirtualMarketDataClient>
        marketDataClient);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

    private:
      friend class BboQuoteEvent;
      friend class QueryBboQuotesEvent;
      BacktesterEventHandler* m_eventHandler;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      std::unordered_set<Security> m_bboQuotes;
  };

  class QueryBboQuotesEvent : public BacktesterEvent {
    public:
      QueryBboQuotesEvent(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::RefType<BacktesterMarketDataService> service);

      virtual void Execute() override;

    private:
      MarketDataService::SecurityMarketDataQuery m_query;
      BacktesterMarketDataService* m_service;
  };

  class BboQuoteEvent : public BacktesterEvent {
    public:
      BboQuoteEvent(Security security, SequencedBboQuote bboQuote,
        Beam::RefType<BacktesterMarketDataService> service);

      virtual void Execute() override;

    private:
      Security m_security;
      SequencedBboQuote m_bboQuote;
      BacktesterMarketDataService* m_service;
  };

  inline BacktesterMarketDataService::BacktesterMarketDataService(
      Beam::RefType<BacktesterEventHandler> eventHandler,
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient)
      : m_eventHandler{eventHandler.Get()},
        m_marketDataClient{marketDataClient.Get()} {}

  inline void BacktesterMarketDataService::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    auto event = std::make_shared<QueryBboQuotesEvent>(query,
      Beam::Ref(*this));
    m_eventHandler->Add(event);
  }

  inline QueryBboQuotesEvent::QueryBboQuotesEvent(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::RefType<BacktesterMarketDataService> service)
      : BacktesterEvent{boost::posix_time::neg_infin},
        m_query{query},
        m_service{service.Get()} {}

  inline void QueryBboQuotesEvent::Execute() {
    if(m_query.GetRange().GetEnd() != Beam::Queries::Sequence::Last()) {
      return;
    }
    if(!m_service->m_bboQuotes.insert(m_query.GetIndex()).second) {
      return;
    }
    auto QUERY_SIZE = 1000;
    auto startTime =
      m_service->m_eventHandler->GetTestEnvironment().GetTimeEnvironment().
      GetTime();
    MarketDataService::SecurityMarketDataQuery backtestQuery;
    backtestQuery.SetIndex(m_query.GetIndex());
    backtestQuery.SetRange(startTime, Beam::Queries::Sequence::Present());
    backtestQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD,
      QUERY_SIZE);
    auto queue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
    m_service->m_marketDataClient->QueryBboQuotes(backtestQuery, queue);
    std::vector<SequencedBboQuote> data;
    Beam::FlushQueue(queue, std::back_inserter(data));
    if(data.empty()) {
      return;
    }
    std::vector<std::shared_ptr<BacktesterEvent>> events;
    for(auto i = data.begin(); i != data.end(); ++i) {
      events.push_back(std::make_shared<BboQuoteEvent>(
        backtestQuery.GetIndex(), std::move(*i), Beam::Ref(*m_service)));
    }
    m_service->m_eventHandler->Add(std::move(events));
  }

  inline BboQuoteEvent::BboQuoteEvent(Security security,
      SequencedBboQuote bboQuote,
      Beam::RefType<BacktesterMarketDataService> service)
      : BacktesterEvent{Beam::Queries::GetTimestamp(*bboQuote)},
        m_security{std::move(security)},
        m_bboQuote{std::move(bboQuote)},
        m_service{service.Get()} {}

  inline void BboQuoteEvent::Execute() {
    m_service->m_eventHandler->GetTestEnvironment().Update(m_security,
      *m_bboQuote);
  }
}

#endif
