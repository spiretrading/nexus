#ifndef SPIRE_PLAYBACK_SPEED_HPP
#define SPIRE_PLAYBACK_SPEED_HPP
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** A ValueModel that represents the playback speed as a Decimal. */
  using PlaybackSpeedModel = ValueModel<Decimal>;

  /** A LocalValueModel that represents the playback speed as a Decimal. */
  using LocalPlaybackSpeedModel = LocalValueModel<Decimal>;
}

#endif
