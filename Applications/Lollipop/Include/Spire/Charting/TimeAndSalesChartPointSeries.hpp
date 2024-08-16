#ifndef SPIRE_TIMEANDSALESCHARTPOINTSERIES_HPP
#define SPIRE_TIMEANDSALESCHARTPOINTSERIES_HPP
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPointSeries.hpp"

namespace Spire {

  /*! \class TimeAndSalesChartPointSeries
      \brief Implements a ChartPointSeries for a TimeAndSales market data feed.
   */
  class TimeAndSalesChartPointSeries : public ChartPointSeries {
    public:

      //! Constructs a TimeAndSalesChartPointSeries.
      /*!
        \param publisher The publisher producing the TimeAndSales.
      */
      TimeAndSalesChartPointSeries(
        const Beam::Publisher<Nexus::TimeAndSale>& publisher);

      virtual ~TimeAndSalesChartPointSeries();

      virtual void Query(ChartValue start, ChartValue end);

      virtual boost::signals2::connection ConnectChartPointAddedSignal(
        const ChartPointAddedSignal::slot_function_type& slot) const;

    private:
      const Beam::Publisher<Nexus::TimeAndSale>* m_publisher;
      Beam::RoutineTaskQueue m_taskQueue;
      mutable ChartPointAddedSignal m_chartPointAddedSignal;

      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
  };
}

#endif
