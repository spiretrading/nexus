#ifndef SPIRE_CHARTPOINTSERIES_HPP
#define SPIRE_CHARTPOINTSERIES_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Charting/Charting.hpp"

namespace Spire {

  /*! \class ChartPointSeries
      \brief Interface for a function or procedure that produces a series of
             ChartPoints.
   */
  class ChartPointSeries : private boost::noncopyable {
    public:

      //! Signals a new ChartPoint was added to this series.
      /*!
        \param point The ChartPoint that was added.
      */
      typedef boost::signals2::signal<void (const ChartPoint& point)>
        ChartPointAddedSignal;

      virtual ~ChartPointSeries();

      //! Submits a query for ChartPoints in a given range.
      /*!
        \param start The start of the range to query (inclusive).
        \param end The end of the range to query (inclusive).
      */
      virtual void Query(ChartValue start, ChartValue end) = 0;

      //! Connects a slot to the ChartPointAddedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      virtual boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPointAddedSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a ChartPointSeries.
      ChartPointSeries();
  };
}

#endif
