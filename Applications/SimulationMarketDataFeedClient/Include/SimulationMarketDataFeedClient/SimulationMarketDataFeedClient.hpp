#ifndef NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#include <ranges>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {

  /**
   * Feeds simulated market data to the MarketDataFeedServer.
   * @param <F> The type of MarketDataFeedClient connected to the
   *        MarketDataFeedServer.
   * @param <T> The type of TimeClient used for timestamps.
   * @param <B> Controls the frequency of BboQuote updates.
   * @param <S> Controls the frequency of TimeAndSale prints.
   */
  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  class SimulationMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient connected to the
       * MarketDataFeedServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<F>;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /** Controls the frequency of BboQuote updates. */
      using BboTimer = Beam::dereference_t<B>;

      /** Controls the frequency of TimeAndSale prints. */
      using TimeAndSaleTimer = Beam::dereference_t<S>;

      /**
       * Constructs a SimulationMarketDataFeedClient.
       * @param securities The list of Securities to simulate market data for.
       * @param venues The venues to use.
       * @param market_data_client The MarketDataClient used to query for
       *        SecurityInfo on each simulated security.
       * @param market_data_feed_client Initializes the MarketDataFeedClient to
       *        send the simulated data through.
       * @param time_client Initializes the TimeClient.
       * @param bbo_timer Initializes the Timer used to update the BboQuotes.
       * @param time_and_sale_timer Initializes the Timer used to update the
       *        TimeAndSales.
       */
      template<Beam::Initializes<F> FF, Beam::Initializes<T> TF,
        Beam::Initializes<B> BF, Beam::Initializes<S> SF>
      SimulationMarketDataFeedClient(const std::vector<Security>& securities,
        const VenueDatabase& venues,
        IsMarketDataClient auto& market_data_client,
        FF&& market_data_feed_client, TF&& time_client, BF&& bbo_timer,
        SF&& time_and_sale_timer);

      ~SimulationMarketDataFeedClient();

      void close();

    private:
      std::vector<SecuritySnapshot> m_securities;
      VenueDatabase m_venues;
      Beam::local_ptr_t<F> m_feed_client;
      Beam::local_ptr_t<T> m_time_client;
      Beam::local_ptr_t<B> m_bbo_timer;
      Beam::local_ptr_t<S> m_time_and_sale_timer;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      void on_bbo_timer_expired(Beam::Timer::Result result);
      void on_time_and_sale_timer_expired(Beam::Timer::Result result);
  };

  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  template<Beam::Initializes<F> FF, Beam::Initializes<T> TF,
    Beam::Initializes<B> BF, Beam::Initializes<S> SF>
  SimulationMarketDataFeedClient<F, T, B, S>::SimulationMarketDataFeedClient(
      const std::vector<Security>& securities, const VenueDatabase& venues,
      IsMarketDataClient auto& market_data_client, FF&& market_data_feed_client,
      TF&& time_client, BF&& bbo_timer, SF&& time_and_sale_timer)
      : m_venues(venues),
        m_feed_client(std::forward<FF>(market_data_feed_client)),
        m_time_client(std::forward<TF>(time_client)),
        m_bbo_timer(std::forward<BF>(bbo_timer)),
        m_time_and_sale_timer(std::forward<SF>(time_and_sale_timer)) {
    for(auto& security : securities) {
      auto info = load_security_info(market_data_client, security);
      if(!info) {
        m_feed_client->add(SecurityInfo(
          security, boost::lexical_cast<std::string>(security), "", 100));
      }
      m_securities.emplace_back();
      auto& snapshot = m_securities.back();
      snapshot.m_security = security;
    }
    try {
      for(auto& snapshot : m_securities) {
        snapshot.m_bbo_quote->m_bid.m_side = Side::BID;
        snapshot.m_bbo_quote->m_bid.m_price =
          ((std::rand() % 100) + 1) * Money::ONE;
        snapshot.m_bbo_quote->m_ask.m_side = Side::ASK;
        snapshot.m_bbo_quote->m_ask.m_price =
          snapshot.m_bbo_quote->m_bid.m_price + Money::CENT;
        snapshot.m_bbo_quote->m_timestamp = m_time_client->get_time();
        auto country =
          m_venues.from(snapshot.m_security.get_venue()).m_country_code;
        auto venues = m_venues.select_all([&] (const auto& entry) {
          return entry.m_country_code == country;
        });
        for(auto& venue : venues) {
          auto ask = BookQuote(venue.m_display_name, false, venue.m_venue,
            make_ask(2 * Money::CENT, 100), m_time_client->get_time());
          snapshot.m_asks.push_back(SequencedSecurityBookQuote(
            SecurityBookQuote(ask, snapshot.m_security),
            Beam::Sequence::FIRST));
          auto bid = BookQuote(venue.m_display_name, false, venue.m_venue,
            make_bid(Money::CENT, 100), m_time_client->get_time());
          snapshot.m_bids.push_back(SequencedSecurityBookQuote(
            SecurityBookQuote(bid, snapshot.m_security),
            Beam::Sequence::FIRST));
        }
      }
      m_bbo_timer->get_publisher().monitor(
        m_tasks.get_slot<Beam::Timer::Result>(std::bind_front(
          &SimulationMarketDataFeedClient::on_bbo_timer_expired, this)));
      m_time_and_sale_timer->get_publisher().monitor(
        m_tasks.get_slot<Beam::Timer::Result>(std::bind_front(
          &SimulationMarketDataFeedClient::on_time_and_sale_timer_expired,
          this)));
      m_bbo_timer->start();
      m_time_and_sale_timer->start();
    } catch(std::exception&) {
      close();
      throw;
    }
  }

  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  SimulationMarketDataFeedClient<F, T, B, S>::
      ~SimulationMarketDataFeedClient() {
    close();
  }

  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  void SimulationMarketDataFeedClient<F, T, B, S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_feed_client->close();
    m_bbo_timer->cancel();
    m_time_and_sale_timer->cancel();
    m_tasks.close();
    m_open_state.close();
  }

  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  void SimulationMarketDataFeedClient<F, T, B, S>::on_bbo_timer_expired(
      Beam::Timer::Result result) {
    for(auto& snapshot : m_securities) {
      snapshot.m_bbo_quote->m_bid.m_size = 100 + (std::rand() % 1000);
      snapshot.m_bbo_quote->m_ask.m_size = 100 + (std::rand() % 1000);
      snapshot.m_bbo_quote->m_timestamp = m_time_client->get_time();
      auto direction = std::rand() % 2;
      if(direction == 1) {
        snapshot.m_bbo_quote->m_bid.m_price += Money::CENT;
        snapshot.m_bbo_quote->m_ask.m_price += Money::CENT;
      } else if(snapshot.m_bbo_quote->m_bid.m_price > Money::CENT) {
        snapshot.m_bbo_quote->m_bid.m_price -= Money::CENT;
        snapshot.m_bbo_quote->m_ask.m_price -= Money::CENT;
      }
      m_feed_client->publish(
        SecurityBboQuote(snapshot.m_bbo_quote, snapshot.m_security));
      for(auto& ask : snapshot.m_asks) {
        ask->m_quote.m_size = 0;
        m_feed_client->publish(SecurityBookQuote(ask, snapshot.m_security));
        ask->m_quote.m_price = snapshot.m_bbo_quote->m_ask.m_price;
        ask->m_quote.m_size = 100 + (std::rand() % 1000);
        ask->m_timestamp = m_time_client->get_time();
        m_feed_client->publish(SecurityBookQuote(ask, snapshot.m_security));
      }
      for(auto& bid : snapshot.m_bids) {
        bid->m_quote.m_size = 0;
        m_feed_client->publish(SecurityBookQuote(bid, snapshot.m_security));
        bid->m_quote.m_price = snapshot.m_bbo_quote->m_bid.m_price;
        bid->m_quote.m_size = 100 + (std::rand() % 1000);
        bid->m_timestamp = m_time_client->get_time();
        m_feed_client->publish(SecurityBookQuote(bid, snapshot.m_security));
      }
    }
    m_bbo_timer->start();
  }

  template<typename F, typename T, typename B, typename S> requires
    IsMarketDataFeedClient<Beam::dereference_t<F>> &&
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsTimer<Beam::dereference_t<B>> &&
          Beam::IsTimer<Beam::dereference_t<S>>
  void SimulationMarketDataFeedClient<F, T, B, S>::
      on_time_and_sale_timer_expired(Beam::Timer::Result result) {
    for(auto& snapshot : m_securities) {
      auto condition =
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@");
      auto price = [&] {
        if(std::rand() % 2 == 0) {
          return snapshot.m_bbo_quote->m_bid.m_price;
        }
        return snapshot.m_bbo_quote->m_ask.m_price;
      }();
      auto time_and_sale = TimeAndSale(m_time_client->get_time(), price, 100,
        condition, snapshot.m_security.get_venue().get_code().get_data(), "M1",
        "M2");
      m_feed_client->publish(
        SecurityTimeAndSale(time_and_sale, snapshot.m_security));
    }
    m_time_and_sale_timer->start();
  }
}

#endif
