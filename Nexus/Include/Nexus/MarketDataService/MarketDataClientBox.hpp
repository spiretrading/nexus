#ifndef NEXUS_MARKET_DATA_CLIENT_BOX_HPP
#define NEXUS_MARKET_DATA_CLIENT_BOX_HPP
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {

  /** Provides a generic interface over an arbitrary MarketDataClient. */
  class MarketDataClientBox {
    public:

      /**
       * Constructs a MarketDataClientBox of a specified type using emplacement.
       * @param <T> The type of market data client to emplace.
       * @param args The arguments to pass to the emplaced market data client.
       */
      template<typename T, typename... Args>
      explicit MarketDataClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a MarketDataClientBox by copying an existing market data
       * client.
       * @param client The client to copy.
       */
      template<typename MarketDataClient>
      explicit MarketDataClientBox(MarketDataClient client);

      explicit MarketDataClientBox(MarketDataClientBox* client);

      explicit MarketDataClientBox(
        const std::shared_ptr<MarketDataClientBox>& client);

      explicit MarketDataClientBox(
        const std::unique_ptr<MarketDataClientBox>& client);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Close();

    private:
      struct VirtualMarketDataClient {
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
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedMarketDataClient final : VirtualMarketDataClient {
        using MarketDataClient = C;
        Beam::GetOptionalLocalPtr<MarketDataClient> m_client;

        template<typename... Args>
        WrappedMarketDataClient(Args&&... args);
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
        void Close() override;
      };
      std::shared_ptr<VirtualMarketDataClient> m_client;
  };

  /**
   * Submits a query for a Security's real-time BookQuotes with snapshot.
   * @param marketDataClient The MarketDataClient used to submit the query.
   * @param security The Security to query for.
   * @param queue The queue that will store the result of the query.
   * @param interruptionPolicy The policy used when the query is interrupted.
   */
  template<typename MarketDataClient>
  Beam::Routines::Routine::Id QueryRealTimeBookQuotesWithSnapshot(
      MarketDataClient&& marketDataClient, const Security& security,
      Beam::ScopedQueueWriter<BookQuote> queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
        Beam::Queries::InterruptionPolicy::BREAK_QUERY) {
    return Beam::Routines::Spawn(
      [marketDataClient = Beam::CapturePtr<MarketDataClient>(marketDataClient),
          security, queue = std::move(queue), interruptionPolicy] () mutable {
        auto snapshot = SecuritySnapshot();
        try {
          snapshot = marketDataClient->LoadSecuritySnapshot(security);
        } catch(const std::exception&) {
          queue.Break(std::current_exception());
          return;
        }
        if(snapshot.m_askBook.empty() && snapshot.m_bidBook.empty()) {
          auto bookQuoteQuery = SecurityMarketDataQuery();
          bookQuoteQuery.SetIndex(security);
          bookQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          bookQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient->QueryBookQuotes(bookQuoteQuery, std::move(queue));
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
          marketDataClient->QueryBookQuotes(bookQuoteQuery, std::move(queue));
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
      MarketDataClient&& marketDataClient, const Security& security,
      Beam::ScopedQueueWriter<MarketQuote> queue,
      Beam::Queries::InterruptionPolicy interruptionPolicy =
        Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE) {
    return Beam::Routines::Spawn(
      [marketDataClient = Beam::CapturePtr<MarketDataClient>(marketDataClient),
          security, queue = std::move(queue), interruptionPolicy] () mutable {
        auto snapshot = SecuritySnapshot();
        try {
          snapshot = marketDataClient->LoadSecuritySnapshot(security);
        } catch(const std::exception&) {
          queue.Break(std::current_exception());
          return;
        }
        if(snapshot.m_marketQuotes.empty()) {
          auto marketQuoteQuery = SecurityMarketDataQuery();
          marketQuoteQuery.SetIndex(security);
          marketQuoteQuery.SetRange(Beam::Queries::Range::RealTime());
          marketQuoteQuery.SetInterruptionPolicy(interruptionPolicy);
          marketDataClient->QueryMarketQuotes(marketQuoteQuery,
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
          marketDataClient->QueryMarketQuotes(marketQuoteQuery,
            std::move(queue));
        }
      });
  }

  template<typename T, typename... Args>
  MarketDataClientBox::MarketDataClientBox(
    std::in_place_type_t<T>, Args&&... args)
      : m_client(std::make_shared<WrappedMarketDataClient<T>>(
          std::forward<Args>(args)...)) {}

  template<typename MarketDataClient>
  MarketDataClientBox::MarketDataClientBox(MarketDataClient client)
    : MarketDataClientBox(std::in_place_type<MarketDataClient>,
        std::move(client)) {}

  inline MarketDataClientBox::MarketDataClientBox(
    MarketDataClientBox* client)
      : MarketDataClientBox(*client) {}

  inline MarketDataClientBox::MarketDataClientBox(
    const std::shared_ptr<MarketDataClientBox>& client)
      : MarketDataClientBox(*client) {}

  inline MarketDataClientBox::MarketDataClientBox(
    const std::unique_ptr<MarketDataClientBox>& client)
      : MarketDataClientBox(*client) {}

  inline void MarketDataClientBox::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  inline void MarketDataClientBox::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  inline SecuritySnapshot MarketDataClientBox::LoadSecuritySnapshot(
      const Security& security) {
    return m_client->LoadSecuritySnapshot(security);
  }

  inline SecurityTechnicals MarketDataClientBox::LoadSecurityTechnicals(
      const Security& security) {
    return m_client->LoadSecurityTechnicals(security);
  }

  inline boost::optional<SecurityInfo> MarketDataClientBox::LoadSecurityInfo(
      const Security& security) {
    return m_client->LoadSecurityInfo(security);
  }

  inline std::vector<SecurityInfo> MarketDataClientBox::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  inline void MarketDataClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  MarketDataClientBox::WrappedMarketDataClient<C>::WrappedMarketDataClient(
    Args&&... args)
      : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  SecuritySnapshot MarketDataClientBox::WrappedMarketDataClient<C>::LoadSecuritySnapshot(
      const Security& security) {
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename C>
  SecurityTechnicals MarketDataClientBox::WrappedMarketDataClient<C>::LoadSecurityTechnicals(
      const Security& security) {
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename C>
  boost::optional<SecurityInfo> MarketDataClientBox::WrappedMarketDataClient<C>::LoadSecurityInfo(
      const Security& security) {
    return m_client->LoadSecurityInfo(security);
  }

  template<typename C>
  std::vector<SecurityInfo> MarketDataClientBox::WrappedMarketDataClient<C>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename C>
  void MarketDataClientBox::WrappedMarketDataClient<C>::Close() {
    m_client->Close();
  }
}

#endif
