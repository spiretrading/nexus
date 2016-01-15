#ifndef NEXUS_MARKETDATAMARKETENTRY_HPP
#define NEXUS_MARKETDATAMARKETENTRY_HPP
#include <Beam/Queries/Sequence.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketEntry
      \brief Keeps track of a Market's data.
   */
  class MarketEntry : private boost::noncopyable {
    public:

      /*! \struct InitialSequences
          \brief Stores the next Sequence to use.
       */
      struct InitialSequences {

        //! The next Sequence to use for an OrderImbalance.
        Beam::Queries::Sequence m_nextOrderImbalanceSequence;
      };

      //! Constructs a MarketEntry.
      /*!
        \param market The market represented.
        \param initialSequences The initial Sequences to use.
      */
      MarketEntry(MarketCode market, const InitialSequences& initialSequences);

      //! Clears market data that originated from a specified source.
      /*!
        \param sourceId The id of the source to clear.
      */
      void Clear(int sourceId);

      //! Publishes an OrderImbalance.
      /*!
        \param orderImbalance The OrderImbalance to publish.
        \param sourceId The id of the source setting the value.
        \return The OrderImbalance to publish.
      */
      boost::optional<SequencedMarketOrderImbalance> PublishOrderImbalance(
        const OrderImbalance& orderImbalance, int sourceId);

    private:
      MarketCode m_market;
      boost::atomic<Beam::Queries::Sequence::Ordinal> m_orderImbalanceSequence;
  };

  inline MarketEntry::MarketEntry(MarketCode market,
      const InitialSequences& initialSequences)
      : m_market(market),
        m_orderImbalanceSequence(
          initialSequences.m_nextOrderImbalanceSequence.GetOrdinal()) {}

  inline void MarketEntry::Clear(int sourceId) {}

  inline boost::optional<SequencedMarketOrderImbalance> MarketEntry::
      PublishOrderImbalance(const OrderImbalance& orderImbalance,
      int sourceId) {
    auto sequence = ++m_orderImbalanceSequence;
    auto sequencedOrderImbalance = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(orderImbalance, m_market),
      Beam::Queries::Sequence(sequence));
    return sequencedOrderImbalance;
  }
}
}

#endif
