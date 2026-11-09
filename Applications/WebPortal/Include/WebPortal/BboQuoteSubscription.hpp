#ifndef NEXUS_BBO_QUOTE_SUBSCRIPTION_HPP
#define NEXUS_BBO_QUOTE_SUBSCRIPTION_HPP
#include <atomic>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /** Supplies the current BBO and subsequent updates for a ticker. */
  class BboQuoteSubscription {
    public:

      /**
       * Subscribes using an authenticated market data client.
       * @param client The client used to query market data.
       * @param ticker The ticker whose BBO updates to receive.
       */
      BboQuoteSubscription(MarketDataClient& client, Ticker ticker);

      ~BboQuoteSubscription();

      /** Returns the subscribed ticker. */
      const Ticker& get_ticker() const;

      /** Returns the updates currently available for the initial snapshot. */
      std::vector<SequencedBboQuote> get_snapshot();

      /** Returns the next update, waiting until one is available. */
      SequencedBboQuote pop();

      /** Returns whether the subscription was cancelled. */
      bool is_closed() const;

      /** Cancels the subscription and wakes any pending reader. */
      void close();

    private:
      Ticker m_ticker;
      std::shared_ptr<Beam::Queue<SequencedBboQuote>> m_updates;
      std::atomic_bool m_is_closed;

      BboQuoteSubscription(const BboQuoteSubscription&) = delete;
      BboQuoteSubscription& operator =(const BboQuoteSubscription&) = delete;
  };
}

#endif
