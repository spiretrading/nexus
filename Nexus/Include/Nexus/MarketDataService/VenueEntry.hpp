#ifndef NEXUS_MARKET_DATA_VENUE_ENTRY_HPP
#define NEXUS_MARKET_DATA_VENUE_ENTRY_HPP
#include <Beam/Queries/Sequencer.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Keeps track of the market data for a specific venue. */
  class VenueEntry {
    public:

      /** Stores the next Sequence to use for each market data type. */
      struct InitialSequences {

        /** The next Sequence to use for an OrderImbalance. */
        Beam::Queries::Sequence m_next_order_imbalance_sequence;
      };

      /**
       * Constructs a VenueEntry.
       * @param venue The venue represented.
       * @param initialSequences The initial Sequences to use.
       */
      VenueEntry(Venue venue, const InitialSequences& initial_sequences);

      /**
       * Clears market data that originated from a specified source.
       * @param source_id The id of the source to clear.
       */
      void clear(int source_id);

      /**
       * Publishes an OrderImbalance.
       * @param imbalance The OrderImbalance to publish.
       * @param source_id The id of the source setting the value.
       * @return The OrderImbalance to publish.
       */
      boost::optional<SequencedVenueOrderImbalance> publish(
        const OrderImbalance& imbalance, int source_id);

    private:
      Venue m_venue;
      Beam::Queries::Sequencer m_order_imbalance_sequencer;
  };

  /**
   * Returns the InitialSequences for a VenueEntry.
   * @param data_store The DataStore to load the InitialSequences from.
   * @param venue The venue to load the InitialSequences for.
   * @return The set of InitialSequences for the specified <i>venue</i>.
   */
  VenueEntry::InitialSequences load_initial_sequences(
      IsHistoricalDataStore auto& data_store, Venue venue) {
    auto query = VenueMarketDataQuery();
    query.SetIndex(venue);
    query.SetRange(Beam::Queries::Range::Total());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    auto initial_sequences = VenueEntry::InitialSequences();
    auto results = data_store.load_order_imbalances(query);
    if(results.empty()) {
      initial_sequences.m_next_order_imbalance_sequence =
        Beam::Queries::Sequence::First();
    } else {
      initial_sequences.m_next_order_imbalance_sequence =
        Beam::Queries::Increment(results.back().GetSequence());
    }
    return initial_sequences;
  }

  inline VenueEntry::VenueEntry(
    Venue venue, const InitialSequences& initial_sequences)
    : m_venue(venue),
      m_order_imbalance_sequencer(
        initial_sequences.m_next_order_imbalance_sequence) {}

  inline void VenueEntry::clear(int source_id) {}

  inline boost::optional<SequencedVenueOrderImbalance> VenueEntry::publish(
      const OrderImbalance& imbalance, int source_id) {
    return m_order_imbalance_sequencer.MakeSequencedValue(imbalance, m_venue);
  }
}

#endif
