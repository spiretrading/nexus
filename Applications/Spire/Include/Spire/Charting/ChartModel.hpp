#ifndef SPIRE_CHART_MODEL_HPP
#define SPIRE_CHART_MODEL_HPP
#include <vector>
#include <Beam/Queries/SnapshotLimit.hpp>
#include <boost/noncopyable.hpp>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Scalar.hpp"

namespace Spire {

  /** Provides a series of candlesticks loaded from a data source. */
  class ChartModel : private boost::noncopyable {
    public:

      //! Signals an update to a candlestick.
      using CandlestickSignal = Signal<void (const Candlestick& candle)>;

      virtual ~ChartModel() = default;

      //! Returns the type of the x-axis.
      virtual Scalar::Type get_x_axis_type() const = 0;

      //! Returns the type of the y-axis.
      virtual Scalar::Type get_y_axis_type() const = 0;

      //! Loads a range of candlesticks and makes them available to query.
      /*!
        \param first The first candlestick in the range to load (inclusive).
        \param last The last candlestick in the range to load (inclusive).
        \param limit Limits the number of candlesticks to load.
      */
      virtual QtPromise<std::vector<Candlestick>> load(Scalar first,
        Scalar last, const Beam::Queries::SnapshotLimit& limit) = 0;

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
