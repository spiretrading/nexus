#ifndef SPIRE_LOCAL_CHART_MODEL_HPP
#define SPIRE_LOCAL_CHART_MODEL_HPP
#include <map>
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
      LocalChartModel(Scalar::Type x_axis_type, Scalar::Type y_axis_type,
        std::vector<Candlestick> candlesticks);

      Scalar::Type get_x_axis_type() const override;

      Scalar::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(Scalar first, Scalar last,
        const Beam::Queries::SnapshotLimit& limit) override;

      //! Inserts candlesticks into the model in sorted order. Duplicate
      //! candlesticks are erased.
      /*
        \param candlesticks A sorted vector of candlesticks to insert into the
                            model.
      */
      void store(const std::vector<Candlestick>& candlesticks);

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      struct CandlestickDomain {
        Scalar m_start;
        Scalar m_end;

        bool operator <(const CandlestickDomain& other) const;
      };

      mutable CandlestickSignal m_candlestick_signal;
      Scalar::Type m_x_axis_type;
      Scalar::Type m_y_axis_type;
      std::map<CandlestickDomain, Candlestick> m_candlesticks;
  };
}

#endif
