#ifndef NEXUS_MARKETDATAVIRTUALMARKETDATACLIENT_HPP
#define NEXUS_MARKETDATAVIRTUALMARKETDATACLIENT_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class VirtualMarketDataClient
      \brief Provides a pure virtual interface to a MarketDataClient.
   */
  class VirtualMarketDataClient : private boost::noncopyable {
    public:
      virtual ~VirtualMarketDataClient();

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue) = 0;

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) = 0;

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedBookQuote>>& queue) = 0;

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedMarketQuote>>& queue) = 0;

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedTimeAndSale>>& queue) = 0;

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) = 0;

      virtual SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) = 0;

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) = 0;

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualMarketDataClient.
      VirtualMarketDataClient();
  };

  /*! \class WrapperMarketDataClient
      \brief Wraps a MarketDataClient providing it with a virtual interface.
      \tparam ClientType The MarketDataClient to wrap.
   */
  template<typename ClientType>
  class WrapperMarketDataClient : public VirtualMarketDataClient {
    public:

      //! The MarketDataClient to wrap.
      typedef typename Beam::TryDereferenceType<ClientType>::type Client;

      //! Constructs a WrapperMarketDataClient.
      /*!
        \param client The MarketDataClient to wrap.
      */
      template<typename MarketDataClientForward>
      WrapperMarketDataClient(MarketDataClientForward&& client);

      virtual ~WrapperMarketDataClient();

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue);

      virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      virtual SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security);

      virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a MarketDataClient into a VirtualMarketDataClient.
  /*!
    \param client The client to wrap.
  */
  template<typename MarketDataClient>
  std::unique_ptr<VirtualMarketDataClient> MakeVirtualMarketDataClient(
      MarketDataClient&& client) {
    return std::make_unique<WrapperMarketDataClient<MarketDataClient>>(
      std::forward<MarketDataClient>(client));
  }

  //! Submits a query for a Security's real-time BookQuotes with snapshot.
  /*!
    \param marketDataClient The MarketDataClient used to submit the query.
    \param security The Security to query for.
    \param queue The queue that will store the result of the query.
    \param interruptionPolicy The policy used when the query is interrupted.
  */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeBookQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::BREAK_QUERY) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue, interruptionPolicy] {
        SecuritySnapshot snapshot;
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception& e) {
          queue->Break(e);
          return;
        }
        if(snapshot.m_askBook.empty() && snapshot.m_bidBook.empty()) {
          SecurityMarketDataQuery bookQuoteQuery;
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, queue);
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& bookQuote : snapshot.m_askBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue->Push(std::move(*bookQuote));
            }
            for(auto& bookQuote : snapshot.m_bidBook) {
              startPoint = std::max(startPoint, bookQuote.GetSequence());
              queue->Push(std::move(*bookQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          SecurityMarketDataQuery bookQuoteQuery;
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(startPoint, Beam::Queries::Sequence::Last());
          bookQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryBookQuotes(bookQuoteQuery, queue);
        }
      });
  }

  //! Submits a query for a Security's real-time MarketQuotes with snapshot.
  /*!
    \param marketDataClient The MarketDataClient used to submit the query.
    \param security The Security to query for.
    \param queue The queue that will store the result of the query.
    \param interruptionPolicy The policy used when the query is interrupted.
  */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeMarketQuotesWithSnapshot(
      MarketDataClient& marketDataClient, const Security& security,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
      Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE) {
    return Beam::Routines::Spawn(
      [&marketDataClient, security, queue, interruptionPolicy] {
        SecuritySnapshot snapshot;
        try {
          snapshot = marketDataClient.LoadSecuritySnapshot(security);
        } catch(const std::exception& e) {
          queue->Break(e);
          return;
        }
        if(snapshot.m_marketQuotes.empty()) {
          SecurityMarketDataQuery marketQuoteQuery;
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery, queue);
        } else {
          auto startPoint = Beam::Queries::Sequence::First();
          try {
            for(auto& marketQuote : snapshot.m_marketQuotes |
                boost::adaptors::map_values) {
              startPoint = std::max(startPoint, marketQuote.GetSequence());
              queue->Push(std::move(*marketQuote));
            }
          } catch(const std::exception&) {
            return;
          }
          startPoint = Beam::Queries::Increment(startPoint);
          SecurityMarketDataQuery marketQuoteQuery;
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(startPoint,
            Beam::Queries::Sequence::Last());
          marketQuoteQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Unlimited());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient.QueryMarketQuotes(marketQuoteQuery, queue);
        }
      });
  }

  inline VirtualMarketDataClient::~VirtualMarketDataClient() {}

  inline VirtualMarketDataClient::VirtualMarketDataClient() {}

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  WrapperMarketDataClient<MarketDataClientType>::WrapperMarketDataClient(
      MarketDataClientForward&& client)
      : m_client(std::forward<MarketDataClientForward>(client)) {}

  template<typename MarketDataClientType>
  WrapperMarketDataClient<MarketDataClientType>::~WrapperMarketDataClient() {}

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    m_client->QueryOrderImbalances(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    m_client->QueryBboQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    m_client->QueryBookQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    m_client->QueryMarketQuotes(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    m_client->QueryTimeAndSales(query, queue);
  }

  template<typename MarketDataClientType>
  SecuritySnapshot WrapperMarketDataClient<MarketDataClientType>::
      LoadSecuritySnapshot(const Security& security) {
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename MarketDataClientType>
  SecurityTechnicals WrapperMarketDataClient<MarketDataClientType>::
      LoadSecurityTechnicals(const Security& security) {
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename MarketDataClientType>
  std::vector<SecurityInfo> WrapperMarketDataClient<MarketDataClientType>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::Open() {
    m_client->Open();
  }

  template<typename MarketDataClientType>
  void WrapperMarketDataClient<MarketDataClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
