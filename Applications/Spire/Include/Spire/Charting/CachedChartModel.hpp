#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include <map>
#include <vector>
#include <Beam/Queries/SnapshotLimit.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartModel.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Spire/QtFuture.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Scalar.hpp"

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
      explicit CachedChartModel(ChartModel& model);

      Scalar::Type get_x_axis_type() const override;

      Scalar::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(Scalar first, Scalar last,
        const Beam::Queries::SnapshotLimit& limit) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      using Interval = boost::icl::continuous_interval<Scalar>;
      struct QueryInfo {
        Scalar m_end;
        std::vector<QtFuture<void>> m_futures;
      };
      ChartModel* m_model;
      LocalChartModel m_cache;
      boost::icl::interval_set<Scalar> m_cached_intervals;
      std::map<Scalar, QueryInfo> m_queried_intervals;

      QtPromise<std::vector<Candlestick>> load_cached_interval(
        const Interval& interval, const Interval& cached_interval,
        const Beam::Queries::SnapshotLimit& limit);
      QtPromise<std::vector<Candlestick>> load_queried_interval(
        const Interval& interval, const Interval& queried_interval,
        const Beam::Queries::SnapshotLimit& limit);
      QtPromise<std::vector<Candlestick>> load_new_interval(
        const Interval& interval,
        const boost::optional<Interval>& cached_interval,
        const boost::optional<Interval>& queried_interval,
        const Beam::Queries::SnapshotLimit& limit);
      boost::optional<Interval> find_leftmost_cached_overlap(
        const Interval& interval) const;
      boost::optional<Interval> find_rightmost_cached_overlap(
        const Interval& interval) const;
      boost::optional<Interval> find_leftmost_queried_overlap(
        const Interval& interval) const;
      boost::optional<Interval> find_rightmost_queried_overlap(
        const Interval& interval) const;
  };
}

#endif
