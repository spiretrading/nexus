#ifndef SPIRE_TRACK_HPP
#define SPIRE_TRACK_HPP
#include "Spire/Login/Login.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Enumerates the application tracks that a user can execute. */
  enum class Track {

    /** The classic Spire track. */
    CLASSIC,

    /** The current Spire version. */
    CURRENT,

    /** The preview track of Spire. */
    PREVIEW
  };

  /** Defines a model over a Track. */
  using TrackModel = ValueModel<Track>;

  /** Defines a local model over a Track. */
  using LocalTrackModel = LocalValueModel<Track>;
}

#endif
