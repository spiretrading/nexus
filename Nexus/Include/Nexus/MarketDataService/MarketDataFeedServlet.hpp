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

namespace Nexus::MarketDataService {

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
      using MarketDataRegistry = Beam::GetTryDereferenceType<R>;
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a MarketDataFeedServlet.
       * @param registry The registry storing all market data originating from
       *        this servlet.
       */
      template<typename RF>
      explicit MarketDataFeedServlet(RF&& registry);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<R> m_registry;
      std::atomic_int m_next_source_id;
      Beam::IO::OpenState m_open_state;

      MarketDataFeedServlet(const MarketDataFeedServlet&) = delete;
      MarketDataFeedServlet& operator =(const MarketDataFeedServlet&) = delete;
      void on_set_security_info_message(
        ServiceProtocolClient& client, const SecurityInfo& info);
      void on_send_market_data_feed_messages(ServiceProtocolClient& client,
        const std::vector<MarketDataFeedMessage>& messages);
  };

  struct MarketDataFeedSession {
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
  template<typename RF>
  MarketDataFeedServlet<C, R>::MarketDataFeedServlet(RF&& registry)
    : m_registry(std::forward<RF>(registry)),
      m_next_source_id(0) {}

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::RegisterServices(Beam::Out<
      Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterMarketDataFeedMessages(Beam::Store(slots));
    Beam::Services::AddMessageSlot<SetSecurityInfoMessage>(
      Store(slots), std::bind_front(
        &MarketDataFeedServlet::on_set_security_info_message, this));
    Beam::Services::AddMessageSlot<SendMarketDataFeedMessages>(
      Store(slots), std::bind_front(
        &MarketDataFeedServlet::on_send_market_data_feed_messages, this));
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_source_id = ++m_next_source_id;
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::HandleClientClosed(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    m_registry->clear(session.m_source_id);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::Close() {
    m_open_state.Close();
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::on_set_security_info_message(
      ServiceProtocolClient& client, const SecurityInfo& info) {
    m_registry->add(info);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::on_send_market_data_feed_messages(
      ServiceProtocolClient& client,
      const std::vector<MarketDataFeedMessage>& messages) {
    auto source_id = client.GetSession().m_source_id;
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
