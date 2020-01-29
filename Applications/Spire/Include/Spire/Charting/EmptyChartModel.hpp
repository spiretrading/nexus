#ifndef SPIRE_EMPTY_CHART_MODEL_HPP
#define SPIRE_EMPTY_CHART_MODEL_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartModel.hpp"

namespace Spire {

  /** Implements an immutable and empty ChartModel containing no data. */
  class EmptyChartModel : public ChartModel {
    public:

      //! Constructs an EmptyChartModel.
      /*!
        \param x_axis_type The type of the x-axis.
        \param y_axis_type The type of the y-axis.
      */
      EmptyChartModel(Scalar::Type x_axis_type, Scalar::Type y_axis_type);

      Scalar::Type get_x_axis_type() const override;

      Scalar::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(Scalar first, Scalar last,
        const Beam::Queries::SnapshotLimit& limit) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      Scalar::Type m_x_axis_type;
      Scalar::Type m_y_axis_type;
  };
}

#endif
