#ifndef SPIRE_DEMO_TIME_AND_SALES_FEED_HPP
#define SPIRE_DEMO_TIME_AND_SALES_FEED_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QRandomGenerator>
#include <QTimer>
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

namespace Spire {

  /**
   * Generates deterministic time and sales for a set of tickers into a
   * TestEnvironment, along with the BBO quotes used to classify them.
   * History up to the present is stored during construction, and subsequent
   * time and sales are published as they become due.
   */
  class DemoTimeAndSalesFeed {
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
         * period by a burst of time and sales sharing a single timestamp that
         * is larger than a single query returns.
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

      /** Specifies the time and sales generated for a ticker. */
      struct Profile {

        /** The ticker to generate time and sales for. */
        Nexus::Ticker m_ticker;

        /** How the ticker's time and sales are spaced in time. */
        Pattern m_pattern;

        /**
         * The time between consecutive time and sales. A non-positive
         * interval generates no time and sales.
         */
        boost::posix_time::time_duration m_interval;
      };

      /**
       * Constructs a DemoTimeAndSalesFeed.
       * @param environment The environment to generate the time and sales
       *        into.
       * @param profiles The time and sales to generate for each ticker.
       * @param start The time the time and sales begin.
       * @param time_client The time client telling the present.
       */
      DemoTimeAndSalesFeed(Beam::Ref<Nexus::TestEnvironment> environment,
        std::vector<Profile> profiles, boost::posix_time::ptime start,
        Beam::TimeClient time_client);

      /**
       * Returns a plain language description of the time and sales generated
       * for a Profile.
       * @param profile The Profile to describe.
       */
      QString describe(const Profile& profile) const;

    private:
      struct Generator {
        Profile m_profile;
        QRandomGenerator m_random;
        int m_price;
        int m_index;
        int m_burst;
        boost::posix_time::ptime m_timestamp;
      };
      Nexus::TestEnvironment* m_environment;
      boost::posix_time::ptime m_start;
      Beam::TimeClient m_time_client;
      std::vector<Generator> m_generators;
      QTimer m_timer;

      DemoTimeAndSalesFeed(const DemoTimeAndSalesFeed&) = delete;
      DemoTimeAndSalesFeed& operator =(const DemoTimeAndSalesFeed&) = delete;
      void advance(Generator& generator) const;
      void store(Generator& generator, boost::posix_time::ptime end);
      void on_timeout();
  };
}

#endif
