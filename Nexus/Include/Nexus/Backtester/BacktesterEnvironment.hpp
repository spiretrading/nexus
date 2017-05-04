#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class BacktesterEnvironment
      \brief Provides all of the services needed to run historical data.
   */
  class BacktesterEnvironment : private boost::noncopyable {
    public:

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
      */
      BacktesterEnvironment(Beam::RefType<
        MarketDataService::VirtualMarketDataClient> marketDataClient,
        boost::posix_time::ptime startTime);

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
        \param endTime The time to stop retrieving data.
      */
      BacktesterEnvironment(Beam::RefType<
        MarketDataService::VirtualMarketDataClient> marketDataClient,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime);

      ~BacktesterEnvironment();

      void Open();

      void Close();

    private:
      friend class BacktesterMarketDataClient;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      TestEnvironment m_testEnvironment;
      Beam::IO::OpenState m_openState;

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);
      void Shutdown();
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient, boost::posix_time::ptime startTime)
      : BacktesterEnvironment{Beam::Ref(marketDataClient),
          std::move(startTime), boost::posix_time::pos_infin} {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime)
      : m_marketDataClient{marketDataClient.Get()},
        m_startTime{std::move(startTime)},
        m_endTime{std::move(endTime)} {}

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    Close();
  }

  inline void BacktesterEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
      m_testEnvironment.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEnvironment::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
  }

  inline void BacktesterEnvironment::Shutdown() {
    m_testEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
