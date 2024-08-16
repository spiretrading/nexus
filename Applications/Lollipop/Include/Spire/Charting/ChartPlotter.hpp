#ifndef SPIRE_CHARTPLOTTER_HPP
#define SPIRE_CHARTPLOTTER_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Charting/Charting.hpp"

namespace Spire {

  /*! \class ChartPlotter
      \brief Interface for a model that produces ChartPlots.
   */
  class ChartPlotter : private boost::noncopyable {
    public:

      //! Signals a ChartPlot was added to a model.
      /*!
        \param plot The ChartPlot that was added.
      */
      typedef boost::signals2::signal<
        void (const std::shared_ptr<ChartPlot>& plot)> ChartPlotAddedSignal;

      //! Signals a ChartPlot was removed from a model.
      /*!
        \param plot The ChartPlot that was removed.
      */
      typedef boost::signals2::signal<
        void (const std::shared_ptr<ChartPlot>& plot)> ChartPlotRemovedSignal;

      virtual ~ChartPlotter();

      //! Connects a slot to the ChartPlotAddedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      virtual boost::signals2::connection ConnectChartPlotAddedSignal(
        const ChartPlotAddedSignal::slot_function_type& slot) const = 0;

      //! Connects a slot to the ChartPlotRemovedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      virtual boost::signals2::connection ConnectChartPlotRemovedSignal(
        const ChartPlotRemovedSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a ChartPlotter.
      ChartPlotter();
  };
}

#endif
