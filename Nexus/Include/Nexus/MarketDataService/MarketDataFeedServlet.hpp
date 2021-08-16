#ifndef NEXUS_MARKET_DATA_FEED_SERVLET_HPP
#define NEXUS_MARKET_DATA_FEED_SERVLET_HPP
#include <atomic>
#include <Beam/IO/OpenState.hpp>
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
       * @param marketDataRegistry The registry storing all market data
       *        originating from this servlet.
       */
      template<typename RF>
      explicit MarketDataFeedServlet(RF&& marketDataRegistry);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<R> m_registry;
      std::atomic_int m_nextSourceId;
      Beam::IO::OpenState m_openState;

      MarketDataFeedServlet(const MarketDataFeedServlet&) = delete;
      MarketDataFeedServlet& operator =(const MarketDataFeedServlet&) = delete;
      void OnSetSecurityInfoMessage(ServiceProtocolClient& client,
        const SecurityInfo& securityInfo);
      void OnSendMarketDataFeedMessages(ServiceProtocolClient& client,
        const std::vector<MarketDataFeedMessage>& messages);
  };

  struct MarketDataFeedSession {
    int m_sourceId;
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
  MarketDataFeedServlet<C, R>::MarketDataFeedServlet(RF&& marketDataRegistry)
    : m_registry(std::forward<RF>(marketDataRegistry)),
      m_nextSourceId(0) {}

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::RegisterServices(Beam::Out<
      Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterMarketDataFeedMessages(Beam::Store(slots));
    Beam::Services::AddMessageSlot<SetSecurityInfoMessage>(Beam::Store(slots),
      std::bind_front(&MarketDataFeedServlet::OnSetSecurityInfoMessage, this));
    Beam::Services::AddMessageSlot<SendMarketDataFeedMessages>(
      Beam::Store(slots), std::bind_front(
        &MarketDataFeedServlet::OnSendMarketDataFeedMessages, this));
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_sourceId = ++m_nextSourceId;
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::HandleClientClosed(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    m_registry->Clear(session.m_sourceId);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::Close() {
    m_openState.Close();
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::OnSetSecurityInfoMessage(
      ServiceProtocolClient& client, const SecurityInfo& securityInfo) {
    m_registry->Add(securityInfo);
  }

  template<typename C, typename R>
  void MarketDataFeedServlet<C, R>::OnSendMarketDataFeedMessages(
      ServiceProtocolClient& client,
      const std::vector<MarketDataFeedMessage>& messages) {
    auto sourceId = client.GetSession().m_sourceId;
    auto visitor = Beam::MakeVariantLambdaVisitor<void>(
      [&] (const SecurityBboQuote& bboQuote) {
        m_registry->PublishBboQuote(bboQuote, sourceId);
      },
      [&] (const SecurityMarketQuote& marketQuote) {
        m_registry->PublishMarketQuote(marketQuote, sourceId);
      },
      [&] (const SecurityBookQuote& bookQuote) {
        m_registry->UpdateBookQuote(bookQuote, sourceId);
      },
      [&] (const SecurityTimeAndSale& timeAndSale) {
        m_registry->PublishTimeAndSale(timeAndSale, sourceId);
      },
      [&] (const MarketOrderImbalance& orderImbalance) {
        m_registry->PublishOrderImbalance(orderImbalance, sourceId);
      });
    for(auto& message : messages) {
      try {
        boost::apply_visitor(visitor, message);
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
  }
}

#endif
