#ifndef NEXUS_MARKETDATACLIENTTESTER_HPP
#define NEXUS_MARKETDATACLIENTTESTER_HPP
#include <vector>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/any.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*! \class MarketDataClientTester
      \brief Tests the MarketDataClient class.
   */
  class MarketDataClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of MarketDataClient.
      using TestMarketDataClient = MarketDataClient<
        Beam::Services::Tests::TestServiceProtocolClientBuilder>;

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
        Beam::Services::Tests::TestServiceProtocolServer::ServiceProtocolClient,
        T>;
      std::shared_ptr<Beam::Services::Tests::TestServerConnection>
        m_serverConnection;
      boost::optional<Beam::Services::Tests::TestServiceProtocolServer>
        m_server;
      std::shared_ptr<Beam::Queue<std::vector<boost::any>>> m_requestQueue;

      std::unique_ptr<ClientEntry> MakeClient();
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
