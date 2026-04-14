#ifndef NEXUS_MARKET_DATA_FEED_SERVLET_HPP
#define NEXUS_MARKET_DATA_FEED_SERVLET_HPP
#include <atomic>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include "Nexus/MarketDataService/MarketDataFeedServices.hpp"

namespace Nexus {

  /**
   * Processes market data feed messages.
   * @param <C> The container instantiating this servlet.
   * @param <R> The registry storing all market data originating from this
   *        servlet.
   */
  template<typename C, typename R>
  class MarketDataFeedServlet {
    public:

      /** The registry storing all market data originating from this servlet. */
      using MarketDataRegistry = Beam::dereference_t<R>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a MarketDataFeedServlet.
       * @param registry The registry storing all market data originating from
       *        this servlet.
       */
      template<Beam::Initializes<R> RF>
      explicit MarketDataFeedServlet(RF&& registry);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      Beam::local_ptr_t<R> m_registry;
      std::atomic_int m_next_source_id;
      Beam::OpenState m_open_state;

      MarketDataFeedServlet(const MarketDataFeedServlet&) = delete;
      MarketDataFeedServlet& operator =(const MarketDataFeedServlet&) = delete;
      void on_set_ticker_info_message(
        ServiceProtocolClient& client, const TickerInfo& info);
      void on_send_market_data_feed_messages(ServiceProtocolClient& client,
        const std::vector<MarketDataFeedMessage>& messages);
  };

  /** Stores session information for a MarketDataFeedServlet client. */
  struct MarketDataFeedSession {

    /** The source ID assigned to the client. */
    int m_source_id;
  };

  template<typename R>
  struct MetaMarketDataFeedServlet {
    using Session = MarketDataFeedSession;

    template<typename C>
    struct apply {
      using type = MarketDataFeedServlet<C, R>;
    };
  };

  template<typename C, typename R>
  template<Beam::Initializes<R> RF>
  MarketDataFeedServlet<C, R>::MarketDataFeedServlet(RF&& registry)
    : m_registry(std::forward<RF>(registry)),
      m_next_source_id(0) {}

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::register_services(Beam::Out<
      Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    register_market_data_feed_messages(Beam::out(slots));
    Beam::add_message_slot<SetTickerInfoMessage>(out(slots), std::bind_front(
      &MarketDataFeedServlet::on_set_ticker_info_message, this));
    Beam::add_message_slot<SendMarketDataFeedMessages>(
      out(slots), std::bind_front(
        &MarketDataFeedServlet::on_send_market_data_feed_messages, this));
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::handle_accept(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    session.m_source_id = ++m_next_source_id;
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::handle_close(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    m_registry->clear(session.m_source_id);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::close() {
    m_open_state.close();
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::on_set_ticker_info_message(
      ServiceProtocolClient& client, const TickerInfo& info) {
    m_registry->add(info);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::on_send_market_data_feed_messages(
      ServiceProtocolClient& client,
      const std::vector<MarketDataFeedMessage>& messages) {
    auto source_id = client.get_session().m_source_id;
    for(auto& message : messages) {
      try {
        boost::apply_visitor([&] (const auto& data) {
          m_registry->publish(data, source_id);
        }, message);
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
  }
}

#endif
