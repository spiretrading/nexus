#ifndef SPIRE_TIMELINE_HPP
#define SPIRE_TIMELINE_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Represents the timeline. */
  struct Timeline {

    /** The start time of the timeline. */
    boost::posix_time::ptime m_start;

    /** The duration of the timeline. */
    boost::posix_time::time_duration m_duration;
  };

  /** A ValueModel over a Timeline. */
  using TimelineModel = ValueModel<Timeline>;

  /** A LocalValueModel over a Timeline. */
  using LocalTimelineModel = LocalValueModel<Timeline>;
}

#endif
