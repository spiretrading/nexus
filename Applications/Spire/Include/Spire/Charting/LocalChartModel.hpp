#ifndef SPIRE_LOCAL_CHART_MODEL_HPP
#define SPIRE_LOCAL_CHART_MODEL_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartModel.hpp"

namespace Spire {

  /** Implements a ChartModel by storing a list of Candlesticks. */
  class LocalChartModel : public ChartModel {
    public:

      //! Constructs a LocalChartModel with an initial list of candlesticks.
      /*!
        \param x_axis_type The type of the x-axis.
        \param y_axis_type The type of the y-axis.
        \param candlesticks The initial list of candlesticks to store.
      */
      LocalChartModel(ChartValue::Type x_axis_type,
        ChartValue::Type y_axis_type, std::vector<Candlestick> candlesticks);

      ChartValue::Type get_x_axis_type() const override;

      ChartValue::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      ChartValue::Type m_x_axis_type;
      ChartValue::Type m_y_axis_type;
      std::vector<Candlestick> m_candlesticks;
  };
}

#endif
