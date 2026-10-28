#ifndef SPIRE_DEMO_MARKET_DATA_FEED_HPP
#define SPIRE_DEMO_MARKET_DATA_FEED_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QTimer>
#include "Nexus/TestEnvironment/TestEnvironment.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Generates deterministic market data for a set of tickers into a
   * TestEnvironment. On construction it stores the history from a start time
   * up to the present directly into the environment's data store; afterwards
   * it publishes each time and sale, and the BBO quote preceding it, through
   * the environment as it falls due, so that history and real time data form
   * one continuous sequence.
   */
  class DemoMarketDataFeed {
    public:

      /** Specifies how a ticker's time and sales are spaced in time. */
      enum class Pattern {

        /** Time and sales occur at a fixed interval. */
        FIXED,

        /**
         * Time and sales occur at a fixed interval, occasionally interrupted
         * by a burst of time and sales sharing a single timestamp.
         */
        BURST,

        /**
         * Time and sales occur at a fixed interval, interrupted at a regular
         * period by a burst of more time and sales sharing a single timestamp
         * than a single query returns.
         */
        LARGE_BURST,

        /**
         * Periods of time and sales at a fixed interval alternate with periods
         * without any time and sales.
         */
        GAP,

        /**
         * No time and sales occur for a period after the start, followed by
         * time and sales at a fixed interval.
         */
        LATE_START
      };

      /** Specifies the market data generated for a ticker. */
      struct Profile {

        /** The ticker to generate market data for. */
        Nexus::Ticker m_ticker;

        /** How the ticker's time and sales are spaced in time. */
        Pattern m_pattern;

        /**
         * The time between consecutive time and sales, a non-positive
         * interval generates no market data.
         */
        boost::posix_time::time_duration m_interval;
      };

      /**
       * Constructs a DemoMarketDataFeed.
       * @param environment The environment to generate the market data into.
       * @param profiles The market data to generate for each ticker.
       * @param start The time market data begins.
       */
      DemoMarketDataFeed(Beam::Ref<Nexus::TestEnvironment> environment,
        std::vector<Profile> profiles, boost::posix_time::ptime start);

      /**
       * Returns a plain language description of the time and sales generated
       * for a Profile.
       * @param profile The Profile to describe.
       */
      QString describe(const Profile& profile) const;

    private:
      struct Generator {
        Profile m_profile;
        quint32 m_seed;
        int m_price;
        int m_next;
        int m_burst;
        boost::posix_time::ptime m_timestamp;
      };
      Nexus::TestEnvironment* m_environment;
      boost::posix_time::ptime m_start;
      std::vector<Generator> m_generators;
      QTimer m_timer;

      DemoMarketDataFeed(const DemoMarketDataFeed&) = delete;
      DemoMarketDataFeed& operator =(const DemoMarketDataFeed&) = delete;
      void advance(Generator& generator) const;
      void store(Generator& generator, boost::posix_time::ptime end);
      void on_timeout();
  };
}

#endif
