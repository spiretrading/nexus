#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include "spire/charting/chart_model.hpp"

namespace Spire {

  //! Provides a caching mechanism for a ChartModel.
  class CachedChartModel : public ChartModel {
    public:

      //! Signals an update to a candlestick.
      using CandlestickSignal = Signal<void (const Candlestick& candle)>;

      //! Represents a range of ChartValues.
      struct ChartRange {
        ChartValue m_start;
        ChartValue m_end;
      };

      //! Constructs a CachedChartModel that uses the supplied model.
      /*!
        \param model The model that supplies the data.
      */
      CachedChartModel(ChartModel& model);

      ChartValue::Type get_x_axis_type() const override;

      ChartValue::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      ChartModel* m_chart_model;
      std::vector<ChartRange> m_ranges;
      std::vector<Candlestick> m_loaded_data;

      QtPromise<std::vector<Candlestick>> load_data(const ChartRange& data);
      std::tuple<std::vector<ChartRange>::iterator,
        std::vector<ChartRange>::iterator> range_search(
        const ChartRange& range);
  };
}

#endif
