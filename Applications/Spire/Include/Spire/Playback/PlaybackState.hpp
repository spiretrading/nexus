#ifndef SPIRE_PLAYBACK_STATE_HPP
#define SPIRE_PLAYBACK_STATE_HPP
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** Enumerates the different play states. */
  enum class PlaybackState {

    /** The replay is currently paused. */
    PAUSED,

    /** The replay is currently playing. */
    REPLAYING,

    /** The replay is displaying data in real time. */
    REAL_TIME
  };

  /** A ValueModel over a PlaybackState. */
  using PlaybackStateModel = ValueModel<PlaybackState>;

  /** A LocalValueModel over a PlaybackState. */
  using LocalPlaybackStateModel = LocalValueModel<PlaybackState>;
}

#endif
