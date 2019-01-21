#ifndef SPIRE_CHART_MODEL_HPP
#define SPIRE_CHART_MODEL_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include "spire/charting/charting.hpp"
#include "spire/spire/qt_promise.hpp"

namespace Spire {

  /** Provides a series of candlesticks loaded from a data source. */
  class ChartModel : private boost::noncopyable {
    public:

      //! Signals an update to a candlestick.
      using CandlestickSignal = Signal<void (const Candlestick& candle)>;

      virtual ~ChartModel() = default;

      //! Loads a range of candlesticks and makes them available to query.
      /*!
        \param first The first candlestick in the range to load (inclusive).
        \param last The last candlestick in the range to load (inclusive).
      */
      virtual QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) = 0;

      //! Connects a slot to the candlestick signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const = 0;

    protected:

      //! Constructs an empty model.
      ChartModel() = default;
  };
}

#endif
