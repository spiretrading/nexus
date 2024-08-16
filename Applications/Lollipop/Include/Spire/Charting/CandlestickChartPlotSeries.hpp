#ifndef SPIRE_CANDLESTICKCHARTPLOTSERIES_HPP
#define SPIRE_CANDLESTICKCHARTPLOTSERIES_HPP
#include <vector>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlotSeries.hpp"
#include "Spire/Charting/ChartValue.hpp"

namespace Spire {

  /*! \class CandlestickChartPlotSeries
      \brief Implements a ChartPlotSeries by building CandlestickChartPlots from
             ChartPoints produced by a ChartPointSeries.
   */
  class CandlestickChartPlotSeries : public ChartPlotSeries {
    public:

      //! Constructs a CandlestickChartPlotSeries.
      /*!
        \param domain The size of the domain of each Candlestick.
        \param chartPointSeries The ChartPointSeries to build the Candlesticks
               from.
      */
      CandlestickChartPlotSeries(ChartValue domain,
        std::unique_ptr<ChartPointSeries> chartPointSeries);

      virtual ~CandlestickChartPlotSeries();

      virtual void Query(ChartValue start, ChartValue end);

      virtual boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPlotAddedSignal::slot_function_type& slot) const;

    private:
      ChartValue m_domain;
      std::unique_ptr<ChartPointSeries> m_chartPointSeries;
      std::vector<std::shared_ptr<CandlestickChartPlot>> m_plots;
      boost::signals2::scoped_connection m_seriesConnection;
      mutable ChartPlotAddedSignal m_chartPlotAddedSignal;

      void OnChartPointAdded(const ChartPoint& chartPoint);
  };
}

#endif
