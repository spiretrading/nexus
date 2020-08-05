#ifndef NEXUS_MARKET_DATA_VIRTUAL_MARKET_DATA_CLIENT_HPP
#define NEXUS_MARKET_DATA_VIRTUAL_MARKET_DATA_CLIENT_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {

  /** Provides a pure virtual interface to a MarketDataClient. */
  class VirtualMarketDataClient : private boost::noncopyable {
    public:
      virtual ~VirtualMarketDataClient() = default;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) = 0;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue) = 0;

      virtual SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) = 0;

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) = 0;

      virtual boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) = 0;

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualMarketDataClient. */
      VirtualMarketDataClient() = default;
  };

  /**
   * Wraps a MarketDataClient providing it with a virtual interface.
   * @param <C> The MarketDataClient to wrap.
   */
  template<typename C>
  class WrapperMarketDataClient final : public VirtualMarketDataClient {
    public:

      /** The MarketDataClient to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperMarketDataClient.
       * @param client The MarketDataClient to wrap.
       */
      template<typename MarketDataClientForward>
      explicit WrapperMarketDataClient(MarketDataClientForward&& client);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) override;

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue) override;

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue) override;

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue) override;

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue) override;

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue) override;

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue) override;

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue) override;

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) override;

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue) override;

      SecuritySnapshot LoadSecuritySnapshot(const Security& security) override;

      SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override;

      boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) override;

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) override;

      void Open() override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a MarketDataClient into a VirtualMarketDataClient.
   * @param client The client to wrap.
   */
  template<typename MarketDataClient>
  std::unique_ptr<VirtualMarketDataClient> MakeVirtualMarketDataClient(
      MarketDataClient&& client) {
    return std::make_unique<WrapperMarketDataClient<MarketDataClient>>(
      std::forward<MarketDataClient>(client));
  }

  /**
   * Submits a query for a Security's real-time BookQuotes with snapshot.
   * @param marketDataClient The MarketDataClient used to submit the query.
   * @param security The Security to query for.
   * @param queue The queue that will store the result of the query.
   * @param interruptionPolicy The policy used when the query is interrupted.
   */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeBookQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::ScopedQueueWriter<BookQuote> queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::BREAK_QUERY) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue = std::move(queue),
          interruptionPolicy] {
        auto snapshot = SecuritySnapshot();
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception&) {
          queue.Break(std::current_exception());
          return;
        }
        if(snapshot.m_askBook.empty() && snapshot.m_bidBook.empty()) {
          auto bookQuoteQuery = SecurityMarketDataQuery();
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, std::move(queue));
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& bookQuote : snapshot.m_askBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue.Push(std::move(*bookQuote));
            }
            for(auto& bookQuote : snapshot.m_bidBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue.Push(std::move(*bookQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          auto bookQuoteQuery = SecurityMarketDataQuery();
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(startPoint, Beam::Queries::Sequence::Last());
          bookQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, std::move(queue));
        }
      });
  }

  /**
   * Submits a query for a Security's real-time MarketQuotes with snapshot.
   * @param marketDataClient The MarketDataClient used to submit the query.
   * @param security The Security to query for.
   * @param queue The queue that will store the result of the query.
   * @param interruptionPolicy The policy used when the query is interrupted.
   */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeMarketQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::ScopedQueueWriter<MarketQuote> queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue = std::move(queue),
          interruptionPolicy] {
        auto snapshot = SecuritySnapshot();
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception&) {
          queue.Break(std::current_exception());
          return;
        }
        if(snapshot.m_marketQuotes.empty()) {
          auto marketQuoteQuery = SecurityMarketDataQuery();
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery,
            std::move(queue));
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& marketQuote : snapshot.m_marketQuotes |
                boost::adaptors::map_values) {
              startPoint = std::max(startPoint, marketQuote.GetSequence());
              queue.Push(std::move(*marketQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          auto marketQuoteQuery = SecurityMarketDataQuery();
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(startPoint,
            Beam::Queries::Sequence::Last());
          marketQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery,
            std::move(queue));
        }
      });
  }

  template<typename C>
  template<typename MarketDataClientForward>
  WrapperMarketDataClient<C>::WrapperMarketDataClient(
    MarketDataClientForward&& client)
    : m_client(std::forward<MarketDataClientForward>(client)) {}

  template<typename C>
  void WrapperMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  void WrapperMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  SecuritySnapshot WrapperMarketDataClient<C>::LoadSecuritySnapshot(
      const Security& security) {
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename C>
  SecurityTechnicals WrapperMarketDataClient<C>::LoadSecurityTechnicals(
      const Security& security) {
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename C>
  boost::optional<SecurityInfo> WrapperMarketDataClient<C>::LoadSecurityInfo(
      const Security& security) {
    return m_client->LoadSecurityInfo(security);
  }

  template<typename C>
  std::vector<SecurityInfo> WrapperMarketDataClient<C>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename C>
  void WrapperMarketDataClient<C>::Open() {
    m_client->Open();
  }

  template<typename C>
  void WrapperMarketDataClient<C>::Close() {
    m_client->Close();
  }
}

#endif
