#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include <boost/icl/continuous_interval.hpp>
#include <boost/icl/interval_set.hpp>
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
      boost::icl::interval_set<ChartValue> m_ranges;
      
      QtPromise<std::vector<Candlestick>> load_from_cache(
        const LoadInfo& info);
      QtPromise<std::vector<Candlestick>> load_from_model(
        const LoadInfo& info);
      void on_data_loaded(const std::vector<Candlestick>& data,
        const boost::icl::continuous_interval<ChartValue>& loaded_range);
      void on_data_loaded(const std::vector<Candlestick>& data,
        const boost::icl::continuous_interval<ChartValue>& loaded_range,
        const Beam::Queries::SnapshotLimit& limit);
  };
}

#endif
