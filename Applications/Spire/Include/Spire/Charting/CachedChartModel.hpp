#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Spire/Charting/LocalChartModel.hpp"

namespace Spire {

  //! Provides a caching mechanism for a ChartModel.
  class CachedChartModel : public ChartModel {
    public:

      //! Signals an update to a candlestick.
      using CandlestickSignal = Signal<void (const Candlestick& candle)>;

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
      enum class RangeType : unsigned char {
        LEFT_CLOSED = 0b0001,
        LEFT_OPEN = 0b0010,
        RIGHT_CLOSED = 0b0100,
        RIGHT_OPEN = 0b1000,
        CLOSED = LEFT_CLOSED | RIGHT_CLOSED,
        OPEN = LEFT_OPEN | RIGHT_OPEN
      };
      struct ChartRange {
        ChartValue m_start;
        ChartValue m_end;
        RangeType m_type;
      };
      struct LoadInfo {
        ChartValue m_first;
        ChartValue m_last;
        ChartValue m_requested_first;
        ChartValue m_requested_last;
        Beam::Queries::SnapshotLimit m_limit;
      };
      mutable CandlestickSignal m_candlestick_signal;
      ChartModel* m_chart_model;
      LocalChartModel m_cache;
      std::vector<ChartRange> m_ranges;

      static bool is_set(RangeType a, RangeType b);
      static bool is_after_range(ChartValue value, const ChartRange& range);
      static bool is_before_range(ChartValue value, const ChartRange& range);
      static bool is_in_range(ChartValue start, ChartValue end,
        const ChartRange& range);
      QtPromise<std::vector<Candlestick>> load_from_cache(
        const LoadInfo& info);
      QtPromise<std::vector<Candlestick>> load_from_model(
        const LoadInfo& info);
      void on_data_loaded(const std::vector<Candlestick>& data,
        const ChartRange& loaded_range);
      void on_data_loaded(const std::vector<Candlestick>& data,
        const ChartRange& loaded_range,
        const Beam::Queries::SnapshotLimit& limit);
      void update_ranges(const ChartRange& new_range);
  };
}

#endif
