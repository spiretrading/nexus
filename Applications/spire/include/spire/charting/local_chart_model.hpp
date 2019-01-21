#ifndef SPIRE_LOCAL_CHART_MODEL_HPP
#define SPIRE_LOCAL_CHART_MODEL_HPP
#include "spire/charting/charting.hpp"
#include "spire/charting/chart_model.hpp"

namespace Spire {

  /** Implements a ChartModel by storing a list of Candlesticks. */
  class LocalChartModel : public ChartModel {
    public:

      //! Constructs a LocalChartModel with an initial list of candlesticks.
      /*!
        \param candlesticks The initial list of candlesticks to store.
      */
      LocalChartModel(std::vector<Candlestick> candlesticks);

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      std::vector<Candlestick> m_candlesticks;
  };
}

#endif
