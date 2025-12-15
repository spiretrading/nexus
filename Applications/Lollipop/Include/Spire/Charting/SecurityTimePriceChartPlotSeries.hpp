#ifndef SPIRE_SECURITY_TIME_PRICE_CHART_PLOT_SERIES_HPP
#define SPIRE_SECURITY_TIME_PRICE_CHART_PLOT_SERIES_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlotSeries.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Implements a ChartPlotSeries from a Security's TimeAndSales. */
  class SecurityTimePriceChartPlotSeries : public ChartPlotSeries {
    public:

      /**
       * Constructs a SecurityTimePriceChartPlotSeries.
       * @param userProfile The user's profile.
       * @param security The Security to plot.
       * @param interval The size of each candlestick.
       */
      SecurityTimePriceChartPlotSeries(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security,
        boost::posix_time::time_duration interval);

      void Query(ChartValue start, ChartValue end) override;

      ChartValue LoadLastCurrentDomain() override;

      boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPlotAddedSignal::slot_function_type& slot) const override;

    private:
      struct TimestampHash {
        std::size_t operator ()(boost::posix_time::ptime value) const;
      };
      struct CandlestickEntry {
        std::shared_ptr<CandlestickChartPlot> m_plot;
        boost::posix_time::ptime m_lastTimestamp;
      };
      UserProfile* m_userProfile;
      Nexus::Security m_security;
      boost::posix_time::time_duration m_interval;
      std::unordered_map<boost::posix_time::ptime, CandlestickEntry,
        TimestampHash> m_candlestickEntries;
      mutable ChartPlotAddedSignal m_chartPlotAddedSignal;
      EventHandler m_eventHandler;
      Beam::RoutineTaskQueue m_taskQueue;

      CandlestickEntry& LoadCandlestick(boost::posix_time::ptime timestamp);
      void OnCandlestickLoaded(
        const Nexus::Candlestick<ChartValue, ChartValue>& candlestick);
      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
  };
}

#endif
