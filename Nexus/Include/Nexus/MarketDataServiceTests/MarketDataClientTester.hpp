#ifndef NEXUS_MARKETDATACLIENTTESTER_HPP
#define NEXUS_MARKETDATACLIENTTESTER_HPP
#include <vector>
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/any.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*! \class MarketDataClientTester
      \brief Tests the MarketDataClient class.
   */
  class MarketDataClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer =
        Beam::Services::ServiceProtocolServer<ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::ServiceProtocolClientBuilder<
        Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of MarketDataClient.
      using TestMarketDataClient =
        MarketDataClient<ServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Connect a MarketDataClient.
      //! Build a SecurityMarketDataQuery for real time data.
      //! Submit the query.
      //! Expect a QuerySecurityBboQuotesService request with the query.
      //! Return an empty snapshot.
      //! Send a BboQuote BQ.
      //! Expect BboQuote BQ.
      void TestRealTimeBboQuoteQuery();

    private:
      struct ClientEntry;
      template<typename T>
      using Request = Beam::Services::RequestToken<
        ServiceProtocolServer::ServiceProtocolClient, T>;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServiceProtocolServer> m_server;
      std::shared_ptr<Beam::Queue<std::vector<boost::any>>> m_requestQueue;

      std::unique_ptr<ClientEntry> MakeClient();
      template<typename T, typename QueryType>
      Request<T> ExpectQuery(const QueryType& query);
      void OnQuerySecurityBboQuotes(Request<QueryBboQuotesService>& request,
        const SecurityMarketDataQuery& query);

      CPPUNIT_TEST_SUITE(MarketDataClientTester);
        CPPUNIT_TEST(TestRealTimeBboQuoteQuery);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
