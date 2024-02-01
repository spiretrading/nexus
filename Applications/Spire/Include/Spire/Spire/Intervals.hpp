#ifndef SPIRE_INTERVALS_HPP
#define SPIRE_INTERVALS_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/icl/interval.hpp>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! The generic type used to specify a bounded value.
  template<typename T>
  using Bound = boost::icl::bounded_value<T>;

  //! The generic type used to specify an interval.
  template<typename T>
  using Interval = boost::icl::continuous_interval<T>;

  //! Specifies a time boundary.
  using TimeBound = Bound<boost::posix_time::ptime>;

  //! Specifies a time interval.
  using TimeInterval = Interval<boost::posix_time::ptime>;
}

#endif
