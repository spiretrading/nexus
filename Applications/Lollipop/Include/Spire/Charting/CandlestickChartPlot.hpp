#ifndef SPIRE_CANDLESTICKCHARTPLOT_HPP
#define SPIRE_CANDLESTICKCHARTPLOT_HPP
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlot.hpp"
#include "Spire/Charting/ChartValue.hpp"

namespace Spire {

  /*! \class CandlestickChartPlot
      \brief Implements a ChartPlot for Candlesticks.
   */
  class CandlestickChartPlot : public ChartPlot {
    public:

      //! The normalized type of Candlestick used.
      typedef Nexus::TechnicalAnalysis::Candlestick<ChartValue, ChartValue>
        CandlestickValue;

      //! Constructs a CandlestickChartPlot.
      /*!
        \param start The start of the domain represented.
        \param end The start of the domain represented.
      */
      CandlestickChartPlot(ChartValue start, ChartValue end);

      //! Constructs a CandlestickChartPlot.
      /*!
        \param value The initial Candlestick to represent.
      */
      CandlestickChartPlot(const CandlestickValue& value);

      virtual ~CandlestickChartPlot();

      //! Returns the Candlestick represented by this plot.
      const CandlestickValue& GetValue() const;

      //! Updates the Candlestick represented by this plot.
      /*!
        \param value The value to update the Candlestick with.
      */
      void Update(ChartValue value);

      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const;

    private:
      CandlestickValue m_value;
      mutable UpdateSignal m_updateSignal;
  };
}

#endif
