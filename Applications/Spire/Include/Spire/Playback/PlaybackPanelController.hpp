#ifndef SPIRE_PLAYBACK_PANEL_CONTROLLER_HPP
#define SPIRE_PLAYBACK_PANEL_CONTROLLER_HPP
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Spire/Playback/Playback.hpp"
#include "Spire/Playback/PlaybackSpeedBox.hpp"
#include "Spire/Playback/SeekBar.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** Implements a controller that handles the replay control logic. */
  class PlaybackPanelController : private QObject {
    public:

      /**
       * Constructs a PlaybackPanelController.
       * @param timeline The timeline that defines the start and duration of
       *        the playback.
       * @param time_client The time client that provides the current time.
       * @param playhead The current position within the timeline.
       * @param speed The playback speed.
       */
      PlaybackPanelController(std::shared_ptr<TimelineModel> timeline,
        Beam::TimeService::TimeClientBox time_client,
        std::shared_ptr<DurationModel> playhead,
        std::shared_ptr<PlaybackSpeedModel> speed);

      /** Returns the timeline model. */
      const std::shared_ptr<TimelineModel>& get_timeline() const;

      /** Returns the time client. */
      Beam::TimeService::TimeClientBox get_time_client() const;

      /** Returns the playhead position. */
      const std::shared_ptr<DurationModel>& get_playhead() const;

      /** Returns the replay speed. */
      const std::shared_ptr<PlaybackSpeedModel>& get_speed() const;

      /** Returns the play state. */
      State get_state() const;

      /** Starts to play . */
      void play();

      /** Pauses the replay. */
      void pause();

      /** Jumps to the start of the replay. */
      void jump_to_start();

      /** Jumps to the end of the replay. */
      void jump_to_end();

      /** Connects a slot to the play state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    private:
      //struct PlayheadModel;
      //std::shared_ptr<PlayheadModel> m_playhead;
      std::shared_ptr<PlaybackSpeedModel> m_speed;
      QTimer m_timer;
      boost::signals2::scoped_connection m_playhead_connection;

      boost::posix_time::time_duration get_duration_until_start() const;
      void on_playhead_update(const boost::posix_time::time_duration& playhead);
      void on_timeout();
  };
}

#endif
