#ifndef SPIRE_CACHED_CHART_MODEL_HPP
#define SPIRE_CACHED_CHART_MODEL_HPP
#include "spire/charting/chart_model.hpp"

namespace Spire {

  //! Provides a caching mechanism for a ChartModel.
  class CachedChartModel : public ChartModel {
    public:

      struct ChartRange {
        ChartValue m_start;
        ChartValue m_end;
      };

      //! Constructs a CachedChartModel that uses the supplied model.
      /*!
        \param model The model that supplies the data.
      */
      CachedChartModel(ChartModel* model);

      ChartValue::Type get_x_axis_type() const override;

      ChartValue::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

    private:
      // pointer or reference?
      ChartModel* m_chart_model;
      std::vector<ChartRange> m_ranges;
      std::vector<Candlestick> m_loaded_data;

      int get_index(const ChartValue& value);
  };
}

#endif
