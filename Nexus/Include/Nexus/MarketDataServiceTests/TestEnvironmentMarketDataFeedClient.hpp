#ifndef NEXUS_TEST_ENVIRONMENT_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TEST_ENVIRONMENT_MARKET_DATA_FEED_CLIENT_HPP
#include <memory>
#include <Beam/Threading/TriggerTimer.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Implements a MarketDataFeedClient for use by the
   * MarketDataServiceTestEnvironment.
   */
  class TestEnvironmentMarketDataFeedClient {
    public:

      /**
       * Constructs a TestEnvironmentMarketDataFeedClient.
       * @param client The MarketDataFeedClient directly connected to the test
       *        environment.
       * @param sampling_timer The timer shared with the client used for
       *        sampling data publications.
       */
      TestEnvironmentMarketDataFeedClient(MarketDataFeedClient client,
        std::shared_ptr<Beam::Threading::TriggerTimer> sampling_timer);
      ~TestEnvironmentMarketDataFeedClient();
      void add(const SecurityInfo& info);
      void publish(const VenueOrderImbalance& imbalance);
      void publish(const SecurityBboQuote& quote);
      void publish(const SecurityBookQuote& quote);
      void publish(const SecurityTimeAndSale& time_and_sale);
      void add_order(const Security& security, Venue venue,
        const std::string& mpid, bool is_primary_mpid, const std::string& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);
      void modify_order_size(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);
      void offset_order_size(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);
      void modify_order_price(
        const std::string& id, Money price, boost::posix_time::ptime timestamp);
      void remove_order(
        const std::string& id, boost::posix_time::ptime timestamp);
      void close();

    private:
      MarketDataFeedClient m_client;
      std::shared_ptr<Beam::Threading::TriggerTimer> m_sampling_timer;

      TestEnvironmentMarketDataFeedClient(
        const TestEnvironmentMarketDataFeedClient&) = delete;
      TestEnvironmentMarketDataFeedClient& operator =(
        const TestEnvironmentMarketDataFeedClient&) = delete;
  };

  inline TestEnvironmentMarketDataFeedClient::
    TestEnvironmentMarketDataFeedClient(MarketDataFeedClient client,
      std::shared_ptr<Beam::Threading::TriggerTimer> sampling_timer)
    : m_client(std::move(client)),
      m_sampling_timer(std::move(sampling_timer)) {}

  inline TestEnvironmentMarketDataFeedClient::
      ~TestEnvironmentMarketDataFeedClient() {
    close();
  }

  inline void TestEnvironmentMarketDataFeedClient::add(
      const SecurityInfo& info) {
    m_client.add(info);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::publish(
      const VenueOrderImbalance& imbalance) {
    m_client.publish(imbalance);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::publish(
      const SecurityBboQuote& quote) {
    m_client.publish(quote);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::publish(
      const SecurityBookQuote& quote) {
    m_client.publish(quote);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::publish(
      const SecurityTimeAndSale& time_and_sale) {
    m_client.publish(time_and_sale);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::add_order(
      const Security& security, Venue venue, const std::string& mpid,
      bool is_primary_mpid, const std::string& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    m_client.add_order(
      security, venue, mpid, is_primary_mpid, id, side, price, size, timestamp);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::modify_order_size(
      const std::string& id, Quantity size,
      boost::posix_time::ptime timestamp) {
    m_client.modify_order_size(id, size, timestamp);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::offset_order_size(
      const std::string& id, Quantity delta,
      boost::posix_time::ptime timestamp) {
    m_client.offset_order_size(id, delta, timestamp);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::modify_order_price(
      const std::string& id, Money price, boost::posix_time::ptime timestamp) {
    m_client.modify_order_price(id, price, timestamp);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::remove_order(
      const std::string& id, boost::posix_time::ptime timestamp) {
    m_client.remove_order(id, timestamp);
    m_sampling_timer->Trigger();
    Beam::Routines::FlushPendingRoutines();
  }

  inline void TestEnvironmentMarketDataFeedClient::close() {
    m_client.close();
    Beam::Routines::FlushPendingRoutines();
  }
}

#endif
