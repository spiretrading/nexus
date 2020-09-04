#ifndef NEXUS_MARKETDATAFEEDSERVLET_HPP
#define NEXUS_MARKETDATAFEEDSERVLET_HPP
#include <atomic>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/MarketDataFeedServices.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataFeedServlet
      \brief Processes market data feed messages.
      \tparam ContainerType The container instantiating this servlet.
      \tparam MarketDataRegistryType The registry storing all market data
              originating from this servlet.
   */
  template<typename ContainerType, typename MarketDataRegistryType>
  class MarketDataFeedServlet : private boost::noncopyable {
    public:

      //! The registry storing all market data originating from this servlet.
      using MarketDataRegistry = Beam::GetTryDereferenceType<
        MarketDataRegistryType>;
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! Constructs a MarketDataFeedServlet.
      /*!
        \param marketDataRegistry The registry storing all market data
               originating from this servlet.
      */
      template<typename MarketDataRegistryForward>
      MarketDataFeedServlet(MarketDataRegistryForward&& marketDataRegistry);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<MarketDataRegistryType> m_registry;
      std::atomic_int m_nextSourceId;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OnResetMessage(ServiceProtocolClient& client,
        MarketDataTypeSet messageTypes);
      void OnSetSecurityInfoMessage(ServiceProtocolClient& client,
        const SecurityInfo& securityInfo);
      void OnSendMarketDataFeedMessages(ServiceProtocolClient& client,
        const std::vector<MarketDataFeedMessage>& messages);
  };

  struct MarketDataFeedSession {
    int m_sourceId;
  };

  template<typename MarketDataRegistryType>
  struct MetaMarketDataFeedServlet {
    using Session = MarketDataFeedSession;
    template<typename ContainerType>
    struct apply {
      using type = MarketDataFeedServlet<ContainerType, MarketDataRegistryType>;
    };
  };

  template<typename ContainerType, typename MarketDataRegistryType>
  template<typename MarketDataRegistryForward>
  MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      MarketDataFeedServlet(MarketDataRegistryForward&& marketDataRegistry)
      : m_registry(std::forward<MarketDataRegistryForward>(marketDataRegistry)),
        m_nextSourceId(0) {}

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      RegisterServices(Beam::Out<
      Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterMarketDataFeedMessages(Beam::Store(slots));
    Beam::Services::AddMessageSlot<ResetDataMessage>(Beam::Store(slots),
      std::bind(&MarketDataFeedServlet::OnResetMessage, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<SetSecurityInfoMessage>(Beam::Store(slots),
      std::bind(&MarketDataFeedServlet::OnSetSecurityInfoMessage, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<SendMarketDataFeedMessages>(
      Beam::Store(slots), std::bind(
      &MarketDataFeedServlet::OnSendMarketDataFeedMessages, this,
      std::placeholders::_1, std::placeholders::_2));
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      HandleClientAccepted(ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    session.m_sourceId = ++m_nextSourceId;
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      HandleClientClosed(ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    m_registry->Clear(session.m_sourceId);
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      Shutdown() {
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      OnResetMessage(ServiceProtocolClient& client,
      MarketDataTypeSet messages) {}

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      OnSetSecurityInfoMessage(ServiceProtocolClient& client,
      const SecurityInfo& securityInfo) {
    m_registry->Add(securityInfo);
  }

  template<typename ContainerType, typename MarketDataRegistryType>
  void MarketDataFeedServlet<ContainerType, MarketDataRegistryType>::
      OnSendMarketDataFeedMessages(ServiceProtocolClient& client,
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
      } catch(...) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
  }
}
}

#endif
