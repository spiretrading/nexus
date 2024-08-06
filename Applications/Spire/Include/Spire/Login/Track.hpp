#ifndef SPIRE_TRACK_HPP
#define SPIRE_TRACK_HPP
#include <QString>
#include "Spire/Login/Login.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

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

  /** Returns the name of the track. */
  const QString& to_text(Track track);

  /** Returns a brief description of the track. */
  const QString& get_description(Track track);
}

#endif
