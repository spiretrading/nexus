#ifndef SPIRE_CHARTPLOTSERIES_HPP
#define SPIRE_CHARTPLOTSERIES_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Charting/Charting.hpp"

namespace Spire {

  /*! \class ChartPlotSeries
      \brief Interface for a function or procedure that produces a series of
             ChartPlots.
   */
  class ChartPlotSeries : private boost::noncopyable {
    public:

      //! Signals a new ChartPlot was added to this series.
      /*!
        \param point The ChartPlot that was added.
      */
      typedef boost::signals2::signal<
        void (const std::shared_ptr<ChartPlot>& plot)> ChartPlotAddedSignal;

      virtual ~ChartPlotSeries();

      //! Submits a query for ChartPlots in a given range.
      /*!
        \param start The start of the range to query (inclusive).
        \param end The end of the range to query (inclusive).
      */
      virtual void Query(ChartValue start, ChartValue end) = 0;

      //! Loads the domain of the last currently available plot.
      virtual ChartValue LoadLastCurrentDomain() = 0;

      //! Connects a slot to the ChartPlotAddedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      virtual boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPlotAddedSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a ChartPlotSeries.
      ChartPlotSeries();
  };
}

#endif
