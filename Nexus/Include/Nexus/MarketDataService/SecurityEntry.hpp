#ifndef NEXUS_MARKET_DATA_SECURITY_ENTRY_HPP
#define NEXUS_MARKET_DATA_SECURITY_ENTRY_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Queries/Sequencer.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {

  /** Keeps track of a Security's market data. */
  class SecurityEntry {
    public:

      /** Stores the next Sequence to use. */
      struct InitialSequences {

        /** The next Sequence to use for a BboQuote. */
        Beam::Queries::Sequence m_nextBboQuoteSequence;

        /** The next Sequence to use for a BookQuote. */
        Beam::Queries::Sequence m_nextBookQuoteSequence;

        /** The next Sequence to use for a MarketQuote. */
        Beam::Queries::Sequence m_nextMarketQuoteSequence;

        /** The next Sequence to use for a TimeAndSale. */
        Beam::Queries::Sequence m_nextTimeAndSaleSequence;
      };

      /**
       * Constructs a SecurityEntry.
       * @param security The Security represented.
       * @param closePrice The closing price.
       * @param initialSequences The initial Sequences to use.
       */
      SecurityEntry(const Security& security, Money closePrice,
        const InitialSequences& initialSequences);

      /** Returns the Security. */
      const Security& GetSecurity() const;

      /** Sets the Security. */
      void SetSecurity(const Security& security);

      /** Returns the most recently published BboQuote. */
      const SequencedSecurityBboQuote& GetBboQuote() const;

      /**
       * Publishes a BboQuote.
       * @param bboQuote The BboQuote to publish.
       * @param sourceId The id of the source setting the value.
       * @return The BboQuote to publish.
       */
      boost::optional<SequencedSecurityBboQuote> PublishBboQuote(
        const BboQuote& bboQuote, int sourceId);

      /**
       * Sets a MarketQuote.
       * @param marketQuote The MarketQuote to set.
       * @param sourceId The id of the source setting the value.
       * @return The MarketQuote to publish.
       */
      boost::optional<SequencedSecurityMarketQuote> PublishMarketQuote(
        const MarketQuote& marketQuote, int sourceId);

      /**
       * Updates a BookQuote.
       * @param delta The BookQuote storing the change.
       * @param sourceId The id of the source setting the value.
       * @return The BookQuote to publish.
       */
      boost::optional<SequencedSecurityBookQuote> UpdateBookQuote(
        const BookQuote& delta, int sourceId);

      /**
       * Publishes a TimeAndSale.
       * @param timeAndSale The TimeAndSale to publish.
       * @param sourceId The id of the source setting the value.
       * @return The TimeAndSale to publish.
       */
      boost::optional<SequencedSecurityTimeAndSale> PublishTimeAndSale(
        const TimeAndSale& timeAndSale, int sourceId);

      /**
       * Returns the Security's current snapshot.
       * @return The real-time snapshot of the <i>security</i>.
       */
      boost::optional<SecuritySnapshot> LoadSnapshot() const;

      /**
       * Clears market data that originated from a specified source.
       * @param sourceId The id of the source to clear.
       */
      void Clear(int sourceId);

    private:
      struct BookQuoteEntry {
        SequencedSecurityBookQuote m_quote;
        int m_sourceId;

        BookQuoteEntry(const SequencedSecurityBookQuote& quote, int sourceId);
      };
      Security m_security;
      Beam::Queries::Sequencer m_bboSequencer;
      Beam::Queries::Sequencer m_marketQuoteSequencer;
      Beam::Queries::Sequencer m_bookQuoteSequencer;
      Beam::Queries::Sequencer m_timeAndSaleSequencer;
      SequencedSecurityBboQuote m_bboQuote;
      SequencedSecurityTimeAndSale m_timeAndSale;
      std::unordered_map<MarketCode, SequencedSecurityMarketQuote>
        m_marketQuotes;
      std::vector<BookQuoteEntry> m_askBook;
      std::vector<BookQuoteEntry> m_bidBook;
  };

  /**
   * Returns the InitialSequences for a SecurityEntry.
   * @param dataStore The DataStore to load the InitialSequences from.
   * @param security The security to load the InitialSequences for.
   * @return The set of InitialSequences for the specified <i>security</i>.
   */
  template<typename DataStore>
  SecurityEntry::InitialSequences LoadInitialSequences(DataStore& dataStore,
      const Security& security) {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Beam::Queries::Range::Total());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    auto initialSequences = SecurityEntry::InitialSequences();
    {
      auto results = dataStore.LoadBboQuotes(query);
      if(results.empty()) {
        initialSequences.m_nextBboQuoteSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextBboQuoteSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    {
      auto results = dataStore.LoadBookQuotes(query);
      if(results.empty()) {
        initialSequences.m_nextBookQuoteSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextBookQuoteSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    {
      auto results = dataStore.LoadMarketQuotes(query);
      if(results.empty()) {
        initialSequences.m_nextMarketQuoteSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextMarketQuoteSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    {
      auto results = dataStore.LoadTimeAndSales(query);
      if(results.empty()) {
        initialSequences.m_nextTimeAndSaleSequence =
          Beam::Queries::Sequence::First();
      } else {
        initialSequences.m_nextTimeAndSaleSequence =
          Beam::Queries::Increment(results.back().GetSequence());
      }
    }
    return initialSequences;
  }

  inline SecurityEntry::BookQuoteEntry::BookQuoteEntry(
    const SequencedSecurityBookQuote& quote, int sourceId)
    : m_quote(quote),
      m_sourceId(sourceId) {}

  inline SecurityEntry::SecurityEntry(const Security& security,
    Money closePrice, const InitialSequences& initialSequences)
    : m_security(security),
      m_bboSequencer(initialSequences.m_nextBboQuoteSequence),
      m_marketQuoteSequencer(initialSequences.m_nextMarketQuoteSequence),
      m_bookQuoteSequencer(initialSequences.m_nextBookQuoteSequence),
      m_timeAndSaleSequencer(initialSequences.m_nextTimeAndSaleSequence) {}

  inline const Security& SecurityEntry::GetSecurity() const {
    return m_security;
  }

  inline void SecurityEntry::SetSecurity(const Security& security) {
    m_security = security;
  }

  inline const SequencedSecurityBboQuote& SecurityEntry::GetBboQuote() const {
    return m_bboQuote;
  }

  inline boost::optional<SequencedSecurityBboQuote> SecurityEntry::
      PublishBboQuote(const BboQuote& bboQuote, int sourceId) {
    auto value = m_bboSequencer.MakeSequencedValue(bboQuote, m_security);
    m_bboQuote = value;
    return value;
  }

  inline boost::optional<SequencedSecurityMarketQuote> SecurityEntry::
      PublishMarketQuote(const MarketQuote& marketQuote, int sourceId) {
    auto value = m_marketQuoteSequencer.MakeSequencedValue(marketQuote,
      m_security);
    m_marketQuotes[marketQuote.m_market] = value;
    return value;
  }

  inline boost::optional<SequencedSecurityBookQuote> SecurityEntry::
      UpdateBookQuote(const BookQuote& delta, int sourceId) {
    auto book = [&] {
      if(delta.m_quote.m_side == Side::ASK) {
        return &m_askBook;
      } else {
        return &m_bidBook;
      }
    }();
    auto entryIterator = Beam::LinearLowerBound(book->begin(), book->end(),
      delta, [] (const auto& lhs, const auto& rhs) {
        return BookQuoteListingComparator(**lhs.m_quote, rhs);
      });
    if(entryIterator == book->end()) {
      if(delta.m_quote.m_size <= 0) {
        return boost::none;
      }
      auto value = m_bookQuoteSequencer.MakeSequencedValue(delta, m_security);
      book->emplace_back(std::move(value), sourceId);
      entryIterator = book->end() - 1;
    } else {
      auto& entry = *entryIterator;
      if((*entry.m_quote)->m_quote.m_price != delta.m_quote.m_price ||
          (*entry.m_quote)->m_mpid != delta.m_mpid) {
        if(delta.m_quote.m_size <= 0) {
          return boost::none;
        }
        if((*entry.m_quote)->m_quote.m_size == 0) {
          auto value = m_bookQuoteSequencer.MakeSequencedValue(delta,
            m_security);
          auto quoteEntry = BookQuoteEntry(std::move(value), sourceId);
          entry = quoteEntry;
        } else {
          auto value = m_bookQuoteSequencer.MakeSequencedValue(delta,
            m_security);
          entryIterator = book->emplace(entryIterator, std::move(value),
            sourceId);
        }
      } else {
        (*entry.m_quote)->m_quote.m_size = std::max<Quantity>(0,
          (*entry.m_quote)->m_quote.m_size + delta.m_quote.m_size);
        (*entry.m_quote)->m_timestamp = delta.m_timestamp;
        entry.m_quote.GetSequence() =
          m_bookQuoteSequencer.IncrementNextSequence(delta.m_timestamp);
        entry.m_sourceId = sourceId;
      }
    }
    return entryIterator->m_quote;
  }

  inline boost::optional<SequencedSecurityTimeAndSale>
      SecurityEntry::PublishTimeAndSale(
        const TimeAndSale& timeAndSale, int sourceId) {
    auto value = m_timeAndSaleSequencer.MakeSequencedValue(
      timeAndSale, m_security);
    m_timeAndSale = value;
    return value;
  }

  inline boost::optional<SecuritySnapshot> SecurityEntry::LoadSnapshot() const {
    if(m_security.GetMarket().IsEmpty()) {
      return boost::none;
    }
    auto snapshot = SecuritySnapshot(m_security);
    snapshot.m_bboQuote = m_bboQuote;
    snapshot.m_timeAndSale = m_timeAndSale;
    snapshot.m_marketQuotes.insert(m_marketQuotes.begin(),
      m_marketQuotes.end());
    for(auto& entry : m_askBook) {
      if((*entry.m_quote)->m_quote.m_size > 0) {
        snapshot.m_askBook.push_back(entry.m_quote);
      }
    }
    for(auto& entry : m_bidBook) {
      if((*entry.m_quote)->m_quote.m_size > 0) {
        snapshot.m_bidBook.push_back(entry.m_quote);
      }
    }
    return snapshot;
  }

  inline void SecurityEntry::Clear(int sourceId) {
    auto askRange = std::remove_if(m_askBook.begin(), m_askBook.end(),
      [&] (auto& bookQuoteEntry) {
        return bookQuoteEntry.m_sourceId == sourceId;
      });
    m_askBook.erase(askRange, m_askBook.end());
    auto bidRange = std::remove_if(m_bidBook.begin(), m_bidBook.end(),
      [&] (auto& bookQuoteEntry) {
        return bookQuoteEntry.m_sourceId == sourceId;
      });
    m_bidBook.erase(bidRange, m_bidBook.end());
  }
}

#endif
