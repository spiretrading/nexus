#ifndef SPIRE_SECURITYTIMEPRICECHARTPLOTSERIES_HPP
#define SPIRE_SECURITYTIMEPRICECHARTPLOTSERIES_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <QTimer>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlotSeries.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class SecurityTimePriceChartPlotSeries
      \brief Implements a ChartPlotSeries from a Security's TimeAndSales.
   */
  class SecurityTimePriceChartPlotSeries : public ChartPlotSeries {
    public:

      //! Constructs a SecurityTimePriceChartPlotSeries.
      /*!
        \param userProfile The user's profile.
        \param security The Security to plot.
        \param interval The size of each candlestick.
      */
      SecurityTimePriceChartPlotSeries(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security,
        const boost::posix_time::time_duration& interval);

      virtual ~SecurityTimePriceChartPlotSeries();

      virtual void Query(ChartValue start, ChartValue end);

      virtual ChartValue LoadLastCurrentDomain();

      virtual boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPlotAddedSignal::slot_function_type& slot) const;

    private:
      struct TimestampHash {
        std::size_t operator ()(const boost::posix_time::ptime& value) const;
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
      QTimer* m_updateTimer;
      QMetaObject::Connection m_timerConnection;
      Beam::TaskQueue m_slotHandler;
      Beam::RoutineTaskQueue m_taskQueue;

      CandlestickEntry& LoadCandlestick(
        const boost::posix_time::ptime& timestamp);
      void OnCandlestickLoaded(const Nexus::TechnicalAnalysis::Candlestick<
        ChartValue, ChartValue>& candlestick);
      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
      void OnUpdateTimer();
  };
}

#endif
