#ifndef NEXUS_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_MARKET_DATA_FEED_CLIENT_HPP
#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Checks if a type implements a MarketDataFeedClient. */
  template<typename T>
  concept IsMarketDataFeedClient = Beam::IsConnection<T> &&
    requires(T& client) {
      client.add(std::declval<const SecurityInfo&>());
      client.publish(std::declval<const VenueOrderImbalance&>());
      client.publish(std::declval<const SecurityBboQuote&>());
      client.publish(std::declval<const SecurityBookQuote&>());
      client.publish(std::declval<const SecurityTimeAndSale&>());
      client.add_order(std::declval<const Security&>(), std::declval<Venue>(),
        std::declval<const std::string&>(), std::declval<bool>(),
        std::declval<const std::string&>(), std::declval<Side>(),
        std::declval<Money>(), std::declval<Quantity>(),
        std::declval<boost::posix_time::ptime>());
      client.modify_order_size(std::declval<const std::string&>(),
        std::declval<Quantity>(), std::declval<boost::posix_time::ptime>());
      client.offset_order_size(std::declval<const std::string&>(),
        std::declval<Quantity>(), std::declval<boost::posix_time::ptime>());
      client.modify_order_price(std::declval<const std::string&>(),
        std::declval<Money>(), std::declval<boost::posix_time::ptime>());
      client.remove_order(std::declval<const std::string&>(),
        std::declval<boost::posix_time::ptime>());
    };

  /** Provides a generic interface over an arbitrary MarketDataFeedClient. */
  class MarketDataFeedClient {
    public:

      /**
       * Constructs a MarketDataFeedClient of a specified type using
       * emplacement.
       * @tparam T The type of market data feed client to emplace.
       * @param args The arguments to pass to the emplaced market data feed
       *        client.
       */
      template<IsMarketDataFeedClient T, typename... Args>
      explicit MarketDataFeedClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a MarketDataFeedClient by referencing an existing market
       * data feed client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<MarketDataFeedClient> T> requires
        IsMarketDataFeedClient<Beam::dereference_t<T>>
      MarketDataFeedClient(T&& client);

      MarketDataFeedClient(const MarketDataFeedClient&) = default;
      MarketDataFeedClient(MarketDataFeedClient&&) = default;

      /**
       * Adds or updates a SecurityInfo.
       * @param info The SecurityInfo to add or update.
       */
      void add(const SecurityInfo& info);

      /**
       * Publishes an OrderImbalance.
       * @param imbalance The OrderImbalance to publish.
       */
      void publish(const VenueOrderImbalance& imbalance);

      /**
       * Publishes a BboQuote.
       * @param quote The BboQuote to publish.
       */
      void publish(const SecurityBboQuote& quote);

      /**
       * Sets a BookQuote.
       * @param quote The BookQuote to set.
       */
      void publish(const SecurityBookQuote& quote);

      /**
       * Publishes a TimeAndSale.
       * @param time_and_sale The TimeAndSale to publish.
       */
      void publish(const SecurityTimeAndSale& time_and_sale);

      /**
       * Adds an order.
       * @param security The Security the order belongs to.
       * @param venue The venue the order was placed on.
       * @param mpid The MPID submitting the order.
       * @param is_primary_mpid Whether the <i>mpid</i> is the <i>venue</i>'s
       *        primary MPID.
       * @param id The order id.
       * @param side The order's Side.
       * @param price The price of the order.
       * @param size The size of the order.
       * @param timestamp The Order's timestamp.
       */
      void add_order(const Security& security, Venue venue,
        const std::string& mpid, bool is_primary_mpid, const std::string& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);

      /**
       * Modifies the size of an order.
       * @param id The order id.
       * @param size The order's new size.
       * @param timestamp The modification's timestamp.
       */
      void modify_order_size(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);

      /**
       * Adds an offset to the size of an order.
       * @param id The order id.
       * @param delta The change in the order's size.
       * @param timestamp The modification's timestamp.
       */
      void offset_order_size(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);

      /**
       * Modifies the price of an order.
       * @param id The order id.
       * @param price The order's new price.
       * @param timestamp The modification's timestamp.
       */
      void modify_order_price(const std::string& id, Money price,
        boost::posix_time::ptime timestamp);

      /**
       * Removes an order.
       * @param id The order id.
       * @param timestamp The modification's timestamp.
       */
      void remove_order(
        const std::string& id, boost::posix_time::ptime timestamp);

      void close();

    private:
      struct VirtualMarketDataFeedClient {
        virtual ~VirtualMarketDataFeedClient() = default;

        virtual void add(const SecurityInfo& info) = 0;
        virtual void publish(const VenueOrderImbalance& imbalance) = 0;
        virtual void publish(const SecurityBboQuote& quote) = 0;
        virtual void publish(const SecurityBookQuote& quote) = 0;
        virtual void publish(const SecurityTimeAndSale& time_and_sale) = 0;
        virtual void add_order(const Security& security, Venue venue,
          const std::string& mpid, bool is_primary_mpid, const std::string& id,
          Side side, Money price, Quantity size,
          boost::posix_time::ptime timestamp) = 0;
        virtual void modify_order_size(const std::string& id, Quantity size,
          boost::posix_time::ptime timestamp) = 0;
        virtual void offset_order_size(const std::string& id, Quantity delta,
          boost::posix_time::ptime timestamp) = 0;
        virtual void modify_order_price(const std::string& id, Money price,
          boost::posix_time::ptime timestamp) = 0;
        virtual void remove_order(
          const std::string& id, boost::posix_time::ptime timestamp) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedMarketDataFeedClient final : VirtualMarketDataFeedClient {
        using MarketDataFeedClient = C;
        Beam::local_ptr_t<MarketDataFeedClient> m_client;

        template<typename... Args>
        WrappedMarketDataFeedClient(Args&&... args);

        void add(const SecurityInfo& info) override;
        void publish(const VenueOrderImbalance& imbalance) override;
        void publish(const SecurityBboQuote& quote) override;
        void publish(const SecurityBookQuote& quote) override;
        void publish(const SecurityTimeAndSale& time_and_sale) override;
        void add_order(const Security& security, Venue venue,
          const std::string& mpid, bool is_primary_mpid, const std::string& id,
          Side side, Money price, Quantity size,
          boost::posix_time::ptime timestamp) override;
        void modify_order_size(const std::string& id, Quantity size,
          boost::posix_time::ptime timestamp) override;
        void offset_order_size(const std::string& id, Quantity delta,
          boost::posix_time::ptime timestamp) override;
        void modify_order_price(const std::string& id, Money price,
          boost::posix_time::ptime timestamp) override;
        void remove_order(
          const std::string& id, boost::posix_time::ptime timestamp) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualMarketDataFeedClient> m_client;
  };

  template<IsMarketDataFeedClient T, typename... Args>
  MarketDataFeedClient::MarketDataFeedClient(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedMarketDataFeedClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<MarketDataFeedClient> T> requires
    IsMarketDataFeedClient<Beam::dereference_t<T>>
  MarketDataFeedClient::MarketDataFeedClient(T&& client)
    : m_client(Beam::make_virtual_ptr<WrappedMarketDataFeedClient<
        std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline void MarketDataFeedClient::add(const SecurityInfo& info) {
    m_client->add(info);
  }

  inline void MarketDataFeedClient::publish(
      const VenueOrderImbalance& imbalance) {
    m_client->publish(imbalance);
  }

  inline void MarketDataFeedClient::publish(const SecurityBboQuote& quote) {
    m_client->publish(quote);
  }

  inline void MarketDataFeedClient::publish(const SecurityBookQuote& quote) {
    m_client->publish(quote);
  }

  inline void MarketDataFeedClient::publish(
      const SecurityTimeAndSale& time_and_sale) {
    m_client->publish(time_and_sale);
  }

  inline void MarketDataFeedClient::add_order(const Security& security,
      Venue venue, const std::string& mpid, bool is_primary_mpid,
      const std::string& id, Side side, Money price, Quantity size,
      boost::posix_time::ptime timestamp) {
    m_client->add_order(
      security, venue, mpid, is_primary_mpid, id, side, price, size, timestamp);
  }

  inline void MarketDataFeedClient::modify_order_size(const std::string& id,
      Quantity size, boost::posix_time::ptime timestamp) {
    m_client->modify_order_size(id, size, timestamp);
  }

  inline void MarketDataFeedClient::offset_order_size(const std::string& id,
      Quantity delta, boost::posix_time::ptime timestamp) {
    m_client->offset_order_size(id, delta, timestamp);
  }

  inline void MarketDataFeedClient::modify_order_price(const std::string& id,
      Money price, boost::posix_time::ptime timestamp) {
    m_client->modify_order_price(id, price, timestamp);
  }

  inline void MarketDataFeedClient::remove_order(
      const std::string& id, boost::posix_time::ptime timestamp) {
    m_client->remove_order(id, timestamp);
  }

  inline void MarketDataFeedClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  MarketDataFeedClient::WrappedMarketDataFeedClient<C>::
    WrappedMarketDataFeedClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::add(
      const SecurityInfo& info) {
    m_client->add(info);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::publish(
      const VenueOrderImbalance& imbalance) {
    m_client->publish(imbalance);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::publish(
      const SecurityBboQuote& quote) {
    m_client->publish(quote);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::publish(
      const SecurityBookQuote& quote) {
    m_client->publish(quote);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::publish(
      const SecurityTimeAndSale& time_and_sale) {
    m_client->publish(time_and_sale);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::add_order(
      const Security& security, Venue venue, const std::string& mpid,
      bool is_primary_mpid, const std::string& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    m_client->add_order(
      security, venue, mpid, is_primary_mpid, id, side, price, size, timestamp);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::modify_order_size(
      const std::string& id, Quantity size,
      boost::posix_time::ptime timestamp) {
    m_client->modify_order_size(id, size, timestamp);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::offset_order_size(
      const std::string& id, Quantity delta,
      boost::posix_time::ptime timestamp) {
    m_client->offset_order_size(id, delta, timestamp);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::
      modify_order_price(const std::string& id, Money price,
        boost::posix_time::ptime timestamp) {
    m_client->modify_order_price(id, price, timestamp);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::remove_order(
      const std::string& id, boost::posix_time::ptime timestamp) {
    m_client->remove_order(id, timestamp);
  }

  template<typename C>
  void MarketDataFeedClient::WrappedMarketDataFeedClient<C>::close() {
    m_client->close();
  }
}

#endif
