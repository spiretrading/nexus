#ifndef SPIRE_EMPTY_CHART_MODEL_HPP
#define SPIRE_EMPTY_CHART_MODEL_HPP
#include "spire/charting/charting.hpp"
#include "spire/charting/chart_model.hpp"

namespace Spire {

  /** Implements an immutable and empty ChartModel containing no data. */
  class EmptyChartModel : public ChartModel {
    public:

      //! Constructs an EmptyChartModel.
      /*!
        \param x_axis_type The type of the x-axis.
        \param y_axis_type The type of the y-axis.
      */
      EmptyChartModel(ChartValue::Type x_axis_type,
        ChartValue::Type y_axis_type);

      ChartValue::Type get_x_axis_type() const override;

      ChartValue::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      ChartValue m_width;
      ChartValue::Type m_x_axis_type;
      ChartValue::Type m_y_axis_type;
  };
}

#endif
