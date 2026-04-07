#ifndef NEXUS_MARKET_DATA_SECURITY_ENTRY_HPP
#define NEXUS_MARKET_DATA_SECURITY_ENTRY_HPP
#include <Beam/Queries/Sequencer.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Keeps track of a Security's market data. */
  class SecurityEntry {
    public:

      /** Stores the next Sequence to use. */
      struct InitialSequences {

        /** The next Sequence to use for a BboQuote. */
        Beam::Sequence m_next_bbo_quote_sequence;

        /** The next Sequence to use for a BookQuote. */
        Beam::Sequence m_next_book_quote_sequence;

        /** The next Sequence to use for a TimeAndSale. */
        Beam::Sequence m_next_time_and_sale_sequence;
      };

      /**
       * Constructs a SecurityEntry.
       * @param security The Security represented.
       * @param venues The venues publishing market data.
       * @param time_zones The database of time zones.
       * @param close The closing price.
       * @param initial_sequences The initial Sequences to use.
       */
      SecurityEntry(Security security, VenueDatabase venues,
        boost::local_time::tz_database time_zones, Money close,
        const InitialSequences& initial_sequences);

      /** Returns the Security. */
      const Security& get_security() const;

      /** Sets the Security. */
      void set_security(const Security& security);

      /** Returns the SecurityTechnicals. */
      const SecurityTechnicals& get_security_technicals() const;

      /**
       * Returns the Security's current snapshot.
       * @return The real-time snapshot of the <i>security</i>.
       */
      boost::optional<SecuritySnapshot> load_snapshot() const;

      /** Returns the most recently published BboQuote. */
      const SequencedSecurityBboQuote& get_bbo_quote() const;

      /**
       * Publishes a BboQuote.
       * @param bbo_quote The BboQuote to publish.
       * @param source_id The id of the source setting the value.
       * @return The BboQuote to publish.
       */
      boost::optional<SequencedSecurityBboQuote> publish(
        const BboQuote& bbo_quote, int source_id);

      /**
       * Updates a BookQuote.
       * @param quote The updated BookQuote to publish.
       * @param source_id The id of the source setting the value.
       * @return The BookQuote to publish.
       */
      boost::optional<SequencedSecurityBookQuote> publish(
        const BookQuote& quote, int source_id);

      /**
       * Publishes a TimeAndSale.
       * @param time_and_sale The TimeAndSale to publish.
       * @param source_id The id of the source setting the value.
       * @return The TimeAndSale to publish.
       */
      boost::optional<SequencedSecurityTimeAndSale> publish(
        const TimeAndSale& time_and_sale, int source_id);

      /**
       * Clears market data that originated from a specified source.
       * @param source_id The id of the source to clear.
       */
      void clear(int source_id);

    private:
      struct BookQuoteEntry {
        SequencedSecurityBookQuote m_quote;
        int m_source_id;

        BookQuoteEntry(const SequencedSecurityBookQuote& quote, int source_id);
      };
      Security m_security;
      VenueDatabase m_venues;
      boost::local_time::tz_database m_time_zones;
      Beam::Sequencer m_bbo_sequencer;
      Beam::Sequencer m_book_quote_sequencer;
      Beam::Sequencer m_time_and_sale_sequencer;
      SecurityTechnicals m_technicals;
      std::string m_market_center;
      Money m_next_close;
      boost::posix_time::ptime m_technicals_reset_time;
      SequencedSecurityBboQuote m_bbo_quote;
      SequencedSecurityTimeAndSale m_time_and_sale;
      std::vector<BookQuoteEntry> m_asks;
      std::vector<BookQuoteEntry> m_bids;

      SecurityEntry(const SecurityEntry&) = delete;
      SecurityEntry& operator =(const SecurityEntry&) = delete;
  };

  /**
   * Returns the InitialSequences for a SecurityEntry.
   * @param data_store The DataStore to load the InitialSequences from.
   * @param security The security to load the InitialSequences for.
   * @return The set of InitialSequences for the specified <i>security</i>.
   */
  SecurityEntry::InitialSequences load_initial_sequences(
      IsHistoricalDataStore auto& data_store, const Security& security) {
    auto query = SecurityMarketDataQuery();
    query.set_index(security);
    query.set_range(Beam::Range::TOTAL);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_tail(1));
    auto initial_sequences = SecurityEntry::InitialSequences();
    {
      auto results = data_store.load_bbo_quotes(query);
      if(results.empty()) {
        initial_sequences.m_next_bbo_quote_sequence = Beam::Sequence::FIRST;
      } else {
        initial_sequences.m_next_bbo_quote_sequence =
          Beam::increment(results.back().get_sequence());
      }
    }
    {
      auto results = data_store.load_book_quotes(query);
      if(results.empty()) {
        initial_sequences.m_next_book_quote_sequence = Beam::Sequence::FIRST;
      } else {
        initial_sequences.m_next_book_quote_sequence =
          Beam::increment(results.back().get_sequence());
      }
    }
    {
      auto results = data_store.load_time_and_sales(query);
      if(results.empty()) {
        initial_sequences.m_next_time_and_sale_sequence = Beam::Sequence::FIRST;
      } else {
        initial_sequences.m_next_time_and_sale_sequence =
          Beam::increment(results.back().get_sequence());
      }
    }
    return initial_sequences;
  }

  inline SecurityEntry::BookQuoteEntry::BookQuoteEntry(
    const SequencedSecurityBookQuote& quote, int source_id)
    : m_quote(quote),
      m_source_id(source_id) {}

  inline SecurityEntry::SecurityEntry(Security security, VenueDatabase venues,
      boost::local_time::tz_database time_zones, Money close,
      const InitialSequences& initial_sequences)
      : m_security(std::move(security)),
        m_venues(std::move(venues)),
        m_time_zones(std::move(time_zones)),
        m_bbo_sequencer(initial_sequences.m_next_bbo_quote_sequence),
        m_book_quote_sequencer(initial_sequences.m_next_book_quote_sequence),
        m_time_and_sale_sequencer(
          initial_sequences.m_next_time_and_sale_sequence) {
    m_market_center = m_venues.from(m_security.get_venue()).m_market_center;
    if(m_market_center.empty()) {
      m_market_center = m_security.get_venue().get_code().get_data();
    }
    m_technicals.m_close = close;
  }

  inline const Security& SecurityEntry::get_security() const {
    return m_security;
  }

  inline void SecurityEntry::set_security(const Security& security) {
    m_security = security;
  }

  inline const SecurityTechnicals&
      SecurityEntry::get_security_technicals() const {
    return m_technicals;
  }

  inline boost::optional<SecuritySnapshot>
      SecurityEntry::load_snapshot() const {
    if(!m_security.get_venue()) {
      return boost::none;
    }
    auto snapshot = SecuritySnapshot(m_security);
    snapshot.m_bbo_quote = m_bbo_quote;
    snapshot.m_time_and_sale = m_time_and_sale;
    for(auto& ask : m_asks) {
      if((*ask.m_quote)->m_quote.m_size > 0) {
        snapshot.m_asks.push_back(ask.m_quote);
      }
    }
    for(auto& bid : m_bids) {
      if((*bid.m_quote)->m_quote.m_size > 0) {
        snapshot.m_bids.push_back(bid.m_quote);
      }
    }
    return snapshot;
  }

  inline const SequencedSecurityBboQuote& SecurityEntry::get_bbo_quote() const {
    return m_bbo_quote;
  }

  inline boost::optional<SequencedSecurityBboQuote> SecurityEntry::publish(
      const BboQuote& bbo_quote, int source_id) {
    if(m_technicals_reset_time == boost::posix_time::not_a_date_time) {
      auto& venue_entry = m_venues.from(m_security.get_venue());
      if(venue_entry.m_venue) {
        auto time_zone =
          m_time_zones.time_zone_from_region(venue_entry.m_time_zone);
        auto reset_time = boost::local_time::local_date_time(
          bbo_quote.m_timestamp, time_zone) + boost::gregorian::days(1);
        reset_time -= reset_time.local_time().time_of_day();
        m_technicals_reset_time = reset_time.utc_time();
      } else {
        m_technicals_reset_time = boost::posix_time::pos_infin;
      }
    }
    if(bbo_quote.m_timestamp >= m_technicals_reset_time) {
      m_technicals.m_volume = 0;
      m_technicals.m_high = Money::ZERO;
      m_technicals.m_low = Money::ZERO;
      m_technicals.m_open = Money::ZERO;
      m_technicals.m_close = m_next_close;
      auto delta =
        bbo_quote.m_timestamp.date() - m_technicals_reset_time.date();
      m_technicals_reset_time += delta;
      if(m_technicals_reset_time <= bbo_quote.m_timestamp) {
        m_technicals_reset_time += boost::gregorian::days(1);
      }
    }
    auto value = m_bbo_sequencer.make_sequenced_value(bbo_quote, m_security);
    m_bbo_quote = value;
    return value;
  }

  inline boost::optional<SequencedSecurityBookQuote> SecurityEntry::publish(
      const BookQuote& quote, int source_id) {
    auto& book = pick(quote.m_quote.m_side, m_asks, m_bids);
    auto i = std::lower_bound(book.begin(), book.end(), quote,
      [] (const auto& lhs, const auto& rhs) {
        return listing_comparator(**lhs.m_quote, rhs);
      });
    if(i == book.end()) {
      if(quote.m_quote.m_size <= 0) {
        return boost::none;
      }
      auto value =
        m_book_quote_sequencer.make_sequenced_value(quote, m_security);
      book.emplace_back(std::move(value), source_id);
      i = book.end() - 1;
    } else {
      auto& entry = *i;
      if((*entry.m_quote)->m_quote.m_price != quote.m_quote.m_price ||
          (*entry.m_quote)->m_mpid != quote.m_mpid) {
        if(quote.m_quote.m_size <= 0) {
          return boost::none;
        }
        if((*entry.m_quote)->m_quote.m_size == 0) {
          auto value =
            m_book_quote_sequencer.make_sequenced_value(quote, m_security);
          auto quote_entry = BookQuoteEntry(std::move(value), source_id);
          entry = quote_entry;
        } else {
          auto value =
            m_book_quote_sequencer.make_sequenced_value(quote, m_security);
          i = book.emplace(i, std::move(value), source_id);
        }
      } else {
        (*entry.m_quote)->m_quote.m_size = std::max<Quantity>(
          0, (*entry.m_quote)->m_quote.m_size + quote.m_quote.m_size);
        (*entry.m_quote)->m_timestamp = quote.m_timestamp;
        entry.m_quote.get_sequence() =
          m_book_quote_sequencer.increment_next_sequence(quote.m_timestamp);
        entry.m_source_id = source_id;
      }
    }
    return i->m_quote;
  }

  inline boost::optional<SequencedSecurityTimeAndSale> SecurityEntry::publish(
      const TimeAndSale& time_and_sale, int source_id) {
    if(m_technicals.m_open == Money::ZERO) {
      m_technicals.m_open = time_and_sale.m_price;
    }
    if(m_technicals.m_high == Money::ZERO ||
        time_and_sale.m_price > m_technicals.m_high) {
      m_technicals.m_high = time_and_sale.m_price;
    }
    if(m_technicals.m_low == Money::ZERO ||
        time_and_sale.m_price < m_technicals.m_low) {
      m_technicals.m_low = time_and_sale.m_price;
    }
    m_technicals.m_volume += time_and_sale.m_size;
    if(time_and_sale.m_market_center == m_market_center) {
      m_next_close = time_and_sale.m_price;
    }
    auto value =
      m_time_and_sale_sequencer.make_sequenced_value(time_and_sale, m_security);
    m_time_and_sale = value;
    return value;
  }

  inline void SecurityEntry::clear(int source_id) {
    std::erase_if(m_asks, [&] (const auto& entry) {
      return entry.m_source_id == source_id;
    });
    std::erase_if(m_bids, [&] (const auto& entry) {
      return entry.m_source_id == source_id;
    });
  }
}

#endif
