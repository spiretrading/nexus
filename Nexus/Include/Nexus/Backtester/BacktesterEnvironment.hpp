#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class BacktesterEnvironment
      \brief Provides all of the services needed to run historical data.
      \tparam MarketDataClientType The type of MarketDataClient used to
              retrieve historical market data.
   */
  template<typename MarketDataClientType>
  class BacktesterEnvironment : private boost::noncopyable {
    public:

      //! The type of MarketDataClient used to retrieve historical market data.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
      */
      template<typename MarketDataClientForward>
      BacktesterEnvironment(MarketDataClientForward&& marketDataClient,
        boost::posix_time::ptime startTime);

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
        \param endTime The time to stop retrieving data.
      */
      template<typename MarketDataClientForward>
      BacktesterEnvironment(MarketDataClientForward&& marketDataClient,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime);

      ~BacktesterEnvironment();

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      TestEnvironment m_testEnvironment;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  BacktesterEnvironment<MarketDataClientType>::BacktesterEnvironment(
      MarketDataClientForward&& marketDataClient,
      boost::posix_time::ptime startTime)
      : BacktesterEnvironment{
          std::forward<MarketDataClientForward>(marketDataClient),
          std::move(startTime), boost::posix_time::pos_infin} {}

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  BacktesterEnvironment<MarketDataClientType>::BacktesterEnvironment(
      MarketDataClientForward&& marketDataClient,
      boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime)
      : m_marketDataClient{
          std::forward<MarketDataClientForward>(marketDataClient)},
        m_startTime{std::move(startTime)},
        m_endTime{std::move(endTime)} {}

  template<typename MarketDataClientType>
  BacktesterEnvironment<MarketDataClientType>::~BacktesterEnvironment() {
    Close();
  }

  template<typename MarketDataClientType>
  void BacktesterEnvironment<MarketDataClientType>::Open() {
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

  template<typename MarketDataClientType>
  void BacktesterEnvironment<MarketDataClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataClientType>
  void BacktesterEnvironment<MarketDataClientType>::Shutdown() {
    m_testEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
