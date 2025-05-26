#ifndef SPIRE_PLAYBACK_SPEED_BOX_HPP
#define SPIRE_PLAYBACK_SPEED_BOX_HPP
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

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
    std::shared_ptr<ValueModel<Decimal>> current, QWidget* parent = nullptr);
}

#endif
