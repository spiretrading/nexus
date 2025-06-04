#ifndef SPIRE_PLAYBACK_SPEED_BOX_HPP
#define SPIRE_PLAYBACK_SPEED_BOX_HPP
#include "Spire/Playback/Playback.hpp"
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel that represents the playback speed as a Decimal. */
  using PlaybackSpeedModel = ValueModel<Decimal>;

  /** A LocalValueModel that represents the playback speed as a Decimal. */
  using LocalPlaybackSpeedModel = LocalValueModel<Decimal>;

  /** An EnumBox specialized for the playback speed. */
  using PlaybackSpeedBox = EnumBox<Decimal>;

  /**
   * Returns a PlaybackSpeedBox with the default playback speed.
   * @param parent The parent widget.
   */
  PlaybackSpeedBox* make_playback_speed_box(QWidget* parent = nullptr);

  /**
   * Returns a PlaybackSpeedBox.
   * @param current The current playback speed.
   * @param parent The parent widget.
   */
  PlaybackSpeedBox* make_playback_speed_box(
    std::shared_ptr<PlaybackSpeedModel> current, QWidget* parent = nullptr);
}

#endif
