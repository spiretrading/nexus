#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include "Spire/Charting/LocalChartModel.hpp"

namespace Spire {

  //! Provides a caching mechanism for a ChartModel.
  class CachedChartModel : public ChartModel {
    public:

      //! Signals an update to a candlestick.
      using CandlestickSignal = Signal<void (const Candlestick& candle)>;

      //! Represents an inclusive range of ChartValues.
      struct ChartRange {

        /** The start value of the range. */
        ChartValue m_start;

        /** The end value of the range. */
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
        ChartValue last, const Beam::Queries::SnapshotLimit& limit) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      ChartModel* m_chart_model;
      std::vector<ChartRange> m_ranges;
      LocalChartModel m_cache;

      void insert_data(const std::vector<Candlestick>& data);
      QtPromise<std::vector<Candlestick>> load_from_cache(ChartValue first,
        ChartValue last, Beam::Queries::SnapshotLimit& limit);
      QtPromise<std::vector<Candlestick>> load_from_model(ChartValue first,
        ChartValue last, Beam::Queries::SnapshotLimit& limit);
      void on_data_loaded(const std::vector<Candlestick>& data,
        ChartValue first, ChartValue last);
      void on_data_loaded(const std::vector<Candlestick>& data,
        ChartValue first, ChartValue last,
        const Beam::Queries::SnapshotLimit& limit);
      void update_ranges(ChartValue first, ChartValue last);
  };
}

#endif
