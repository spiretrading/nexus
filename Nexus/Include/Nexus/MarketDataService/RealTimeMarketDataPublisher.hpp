#ifndef NEXUS_MARKETDATAREALTIMEMARKETDATAPUBLISHER_HPP
#define NEXUS_MARKETDATAREALTIMEMARKETDATAPUBLISHER_HPP
#include <Beam/Queues/QueuePublisher.hpp>
#include <Beam/Queues/SnapshotPublisher.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class RealTimeMarketDataPublisher
      \brief Provides Publishers for real-time market data.
   */
  template<typename MarketDataClientType>
  class RealTimeMarketDataPublisher : private boost::noncopyable {
    public:

      //! The type of MarketDataClient to query.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! Constructs a RealTimeMarketDataPublisher.
      /*!
        \param marketDataClient The MarketDataClient to query.
      */
      template<typename MarketDataClientForward>
      RealTimeMarketDataPublisher(MarketDataClientForward&& marketDataClient);

      //! Returns a Publisher for BboQuotes.
      /*!
        \param security The Security to get the BboQuotes for.
        \return A BboQuote Publisher for the specified <i>security</i>.
      */
      const Beam::SnapshotPublisher<BboQuote, BboQuote>& GetBboQuotePublisher(
        const Security& security);

    private:
      using BboQuotePublisher =
        Beam::QueuePublisher<Beam::StatePublisher<BboQuote>>;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<BboQuotePublisher>> m_bboQuotePublishers;
  };

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  RealTimeMarketDataPublisher<MarketDataClientType>::
      RealTimeMarketDataPublisher(MarketDataClientForward&& marketDataClient)
      : m_marketDataClient{std::forward<MarketDataClientForward>(
          marketDataClient)} {}

  template<typename MarketDataClientType>
  const Beam::SnapshotPublisher<BboQuote, BboQuote>&
      RealTimeMarketDataPublisher<MarketDataClientType>::GetBboQuotePublisher(
      const Security& security) {
    auto publisher = m_bboQuotePublishers.GetOrInsert(security,
      [&] {
        auto queue = std::make_shared<Beam::Queue<BboQuote>>();
        QueryRealTimeWithSnapshot(security, *m_marketDataClient, queue);
        auto publisher = std::make_shared<BboQuotePublisher>(queue);
        return publisher;
      });
    return *publisher;
  }
}
}

#endif
