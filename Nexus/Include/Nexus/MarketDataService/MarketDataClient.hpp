#ifndef NEXUS_MARKET_DATA_CLIENT_HPP
#define NEXUS_MARKET_DATA_CLIENT_HPP
#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Routine.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Checks if a type implements a MarketDataClient. */
  template<typename T>
  concept IsMarketDataClient = Beam::IsConnection<T> && requires(T& client) {
    client.query(std::declval<const VenueMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<SequencedOrderImbalance>>());
    client.query(std::declval<const VenueMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<OrderImbalance>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<SequencedBboQuote>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<BboQuote>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<SequencedBookQuote>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<BookQuote>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<SequencedTimeAndSale>>());
    client.query(std::declval<const SecurityMarketDataQuery&>(),
      std::declval<Beam::ScopedQueueWriter<TimeAndSale>>());
    { client.query(std::declval<const SecurityInfoQuery&>()) } ->
        std::same_as<std::vector<SecurityInfo>>;
    { client.load_snapshot(std::declval<const Security&>()) } ->
        std::same_as<SecuritySnapshot>;
    { client.load_technicals(std::declval<const Security&>()) } ->
        std::same_as<SecurityTechnicals>;
    { client.load_security_info_from_prefix(
        std::declval<const std::string&>()) } ->
          std::same_as<std::vector<SecurityInfo>>;
  };

  /** Provides a generic interface over an arbitrary MarketDataClient. */
  class MarketDataClient {
    public:

      /**
       * Constructs a MarketDataClient of a specified type using emplacement.
       * @tparam T The type of market data client to emplace.
       * @param args The arguments to pass to the emplaced market data client.
       */
      template<IsMarketDataClient T, typename... Args>
      explicit MarketDataClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a MarketDataClient by referencing an existing market data
       * client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<MarketDataClient> T> requires
        IsMarketDataClient<Beam::dereference_t<T>>
      MarketDataClient(T&& client);

      MarketDataClient(const MarketDataClient&) = default;
      MarketDataClient(MarketDataClient&&) = default;

      /**
       * Submits a query for a Venue's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      /**
       * Submits a query for a Venue's OrderImbalances.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      /**
       * Submits a query for a Security's BboQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      /**
       * Submits a query for a Security's BookQuotes.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      /**
       * Submits a query for a Security's TimeAndSales.
       * @param query The query to submit.
       * @param queue The queue that will store the result of the query.
       */
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      /**
       * Queries for all SecurityInfo objects that are within a region.
       * @param query The query to submit.
       * @return The list of SecurityInfo objects that match the <i>query</i>.
       */
      std::vector<SecurityInfo> query(const SecurityInfoQuery& query);

      /**
       * Loads a Security's real-time snapshot.
       * @param security The Security whose SecuritySnapshot is to be loaded.
       * @return The real-time snapshot of the specified <i>security</i>.
       */
      SecuritySnapshot load_snapshot(const Security& security);

      /**
       * Loads the SecurityTechnicals for a specified Security.
       * @param security The Security whose SecurityTechnicals is to be loaded.
       * @return The SecurityTechnicals for the specified <i>security</i>.
       */
      SecurityTechnicals load_technicals(const Security& security);

      /**
       * Loads SecurityInfo objects that match a prefix.
       * @param prefix The prefix to search for.
       * @return The list of SecurityInfo objects that match the <i>prefix</i>.
       */
      std::vector<SecurityInfo> load_security_info_from_prefix(
        const std::string& prefix);

      void close();

    private:
      struct VirtualMarketDataClient {
        virtual ~VirtualMarketDataClient() = default;

        virtual void query(const VenueMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) = 0;
        virtual void query(const VenueMarketDataQuery& query,
          Beam::ScopedQueueWriter<OrderImbalance> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedBboQuote> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<BboQuote> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedBookQuote> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<BookQuote> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) = 0;
        virtual void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<TimeAndSale> queue) = 0;
        virtual std::vector<SecurityInfo> query(
          const SecurityInfoQuery& query) = 0;
        virtual SecuritySnapshot load_snapshot(const Security& security) = 0;
        virtual SecurityTechnicals load_technicals(
          const Security& security) = 0;
        virtual std::vector<SecurityInfo> load_security_info_from_prefix(
          const std::string& prefix) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedMarketDataClient final : VirtualMarketDataClient {
        using MarketDataClient = C;
        Beam::local_ptr_t<MarketDataClient> m_client;

        template<typename... Args>
        WrappedMarketDataClient(Args&&... args);

        void query(const VenueMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) override;
        void query(const VenueMarketDataQuery& query,
          Beam::ScopedQueueWriter<OrderImbalance> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedBboQuote> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<BboQuote> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedBookQuote> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<BookQuote> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) override;
        void query(const SecurityMarketDataQuery& query,
          Beam::ScopedQueueWriter<TimeAndSale> queue) override;
        std::vector<SecurityInfo> query(
          const SecurityInfoQuery& query) override;
        SecuritySnapshot load_snapshot(const Security& security) override;
        SecurityTechnicals load_technicals(const Security& security) override;
        std::vector<SecurityInfo> load_security_info_from_prefix(
          const std::string& prefix) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualMarketDataClient> m_client;
  };

  /**
   * Submits a query for a Security's real-time BookQuotes with snapshot.
   * @param client The MarketDataClient used to submit the query.
   * @param security The Security to query for.
   * @param queue The queue that will store the result of the query.
   * @param interruption_policy The policy used when the query is interrupted.
   */
  Beam::Routine::Id query_real_time_with_snapshot(
      IsMarketDataClient auto& client, Security security,
      Beam::ScopedQueueWriter<BookQuote> queue,
      Beam::InterruptionPolicy interruption_policy =
        Beam::InterruptionPolicy::BREAK_QUERY) {
    return Beam::spawn([&client, security = std::move(security),
          queue = std::move(queue), interruption_policy] () mutable {
        auto snapshot = SecuritySnapshot();
        try {
          snapshot = client.load_snapshot(security);
        } catch(const std::exception&) {
          queue.close(std::current_exception());
          return;
        }
        if(snapshot.m_asks.empty() && snapshot.m_bids.empty()) {
          auto query = SecurityMarketDataQuery();
          query.set_index(security);
          query.set_range(Beam::Range::REAL_TIME);
          query.set_interruption_policy(interruption_policy);
          client.query(query, std::move(queue));
        } else {
          auto start = Beam::Sequence::FIRST;
          try {
            for(auto& quote : snapshot.m_asks) {
              start = std::max(start, quote.get_sequence());
              queue.push(std::move(*quote));
            }
            for(auto& quote : snapshot.m_bids) {
              start = std::max(start, quote.get_sequence());
              queue.push(std::move(*quote));
            }
          } catch(const std::exception&) {
            return;
          }
          start = Beam::increment(start);
          auto query = SecurityMarketDataQuery();
          query.set_index(security);
          query.set_range(start, Beam::Sequence::LAST);
          query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
          query.set_interruption_policy(interruption_policy);
          client.query(query, std::move(queue));
        }
      });
  }

  /**
   * Submits a query for real time BboQuotes with a snapshot.
   * @param client The MarketDataClient to submit the query to.
   * @param security The Security to query.
   * @param queue The Queue to write to.
   */
  Beam::Routine::Id query_real_time_with_snapshot(
      IsMarketDataClient auto& client, Security security,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    return Beam::spawn(
      [=, &client, security = std::move(security),
          queue = std::move(queue)] () mutable {
        auto snapshot_queue =
          std::make_shared<Beam::Queue<SequencedBboQuote>>();
        client.query(Beam::make_latest_query(security), snapshot_queue);
        auto snapshot = SequencedBboQuote();
        try {
          snapshot = snapshot_queue->pop();
          queue.push(std::move(*snapshot));
        } catch(const Beam::PipeBrokenException&) {
          return;
        }
        auto continuation_query = SecurityMarketDataQuery();
        continuation_query.set_index(std::move(security));
        continuation_query.set_range(
          Beam::increment(snapshot.get_sequence()), Beam::Sequence::LAST);
        continuation_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
        continuation_query.set_interruption_policy(
          Beam::InterruptionPolicy::IGNORE_CONTINUE);
        client.query(continuation_query, std::move(queue));
      });
  }

  /**
   * Submits a query to retrieve the SecurityInfo for a single security.
   * @param client The MarketDataClient to submit the query to.
   * @param security The Security to query.
   * @return The SecurityInfo for the given <i>security</i>.
   */
  boost::optional<SecurityInfo> load_security_info(
      IsMarketDataClient auto& client, const Security& security) {
    auto result = client.query(make_security_info_query(security));
    if(!result.empty()) {
      return result.front();
    }
    return boost::none;
  }

  template<IsMarketDataClient T, typename... Args>
  MarketDataClient::MarketDataClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedMarketDataClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<MarketDataClient> T> requires
    IsMarketDataClient<Beam::dereference_t<T>>
  MarketDataClient::MarketDataClient(T&& client)
    : m_client(Beam::make_virtual_ptr<WrappedMarketDataClient<
        std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline void MarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_client->query(query, std::move(queue));
  }

  inline void MarketDataClient::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_client->query(query, std::move(queue));
  }

  inline std::vector<SecurityInfo> MarketDataClient::query(
      const SecurityInfoQuery& query) {
    return m_client->query(query);
  }

  inline SecuritySnapshot MarketDataClient::load_snapshot(
      const Security& security) {
    return m_client->load_snapshot(security);
  }

  inline SecurityTechnicals MarketDataClient::load_technicals(
      const Security& security) {
    return m_client->load_technicals(security);
  }

  inline std::vector<SecurityInfo> MarketDataClient::
      load_security_info_from_prefix(const std::string& prefix) {
    return m_client->load_security_info_from_prefix(prefix);
  }

  inline void MarketDataClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  MarketDataClient::WrappedMarketDataClient<C>::WrappedMarketDataClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_client->query(query, std::move(queue));
  }

  template<typename C>
  std::vector<SecurityInfo> MarketDataClient::WrappedMarketDataClient<C>::
      query(const SecurityInfoQuery& query) {
    return m_client->query(query);
  }

  template<typename C>
  SecuritySnapshot MarketDataClient::WrappedMarketDataClient<C>::
      load_snapshot(const Security& security) {
    return m_client->load_snapshot(security);
  }

  template<typename C>
  SecurityTechnicals MarketDataClient::WrappedMarketDataClient<C>::
      load_technicals(const Security& security) {
    return m_client->load_technicals(security);
  }

  template<typename C>
  std::vector<SecurityInfo> MarketDataClient::WrappedMarketDataClient<C>::
      load_security_info_from_prefix(const std::string& prefix) {
    return m_client->load_security_info_from_prefix(prefix);
  }

  template<typename C>
  void MarketDataClient::WrappedMarketDataClient<C>::close() {
    m_client->close();
  }
}

#endif
