#ifndef SPIRE_CHARTPLOT_HPP
#define SPIRE_CHARTPLOT_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Charting/Charting.hpp"

namespace Spire {

  /*! \class ChartPlot
      \brief Represents a single plot (which may consist of multiple points) on
             a chart.
   */
  class ChartPlot : private boost::noncopyable {
    public:

      //! Signals an update to this plot.
      typedef boost::signals2::signal<void ()> UpdateSignal;

      virtual ~ChartPlot();

      //! Connects a slot to the UpdateSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a ChartPlot.
      ChartPlot();
  };
}

#endif
