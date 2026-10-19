#ifndef SPIRE_REPLAY_SESSION_HPP
#define SPIRE_REPLAY_SESSION_HPP
#include <memory>
#include <Beam/TimeService/TimeClient.hpp>
#include <QTimer>
#include "Spire/Playback/PlaybackSpeed.hpp"
#include "Spire/Playback/PlaybackState.hpp"
#include "Spire/Playback/Timeline.hpp"
#include "Spire/Ui/DurationBox.hpp"

namespace Spire {

  /**
   * Represents a single replay, advancing its playhead along a timeline at the
   * playback speed and tracking whether it is paused, replaying or in real
   * time.
   */
  class ReplaySession {
    public:

      /**
       * Constructs a ReplaySession.
       * @param timeline The timeline being played back.
       * @param playhead The position within the timeline, limited to the
       *        timeline and the present.
       * @param speed The playback speed, 1x if it is not positive.
       * @param time_client Paces the playhead and tells the present.
       */
      ReplaySession(Timeline timeline,
        boost::posix_time::time_duration playhead, Decimal speed,
        Beam::TimeClient time_client);

      /** Returns the timeline being played back. */
      const std::shared_ptr<TimelineModel>& get_timeline() const;

      /** Returns the position within the timeline. */
      const std::shared_ptr<DurationModel>& get_playhead() const;

      /**
       * Returns the absolute position, the timeline's start plus the playhead.
       */
      const std::shared_ptr<ValueModel<boost::posix_time::ptime>>&
        get_position() const;

      /** Returns the playback speed. */
      const std::shared_ptr<PlaybackSpeedModel>& get_speed() const;

      /** Returns the play state. */
      const std::shared_ptr<PlaybackStateModel>& get_state() const;

      /** Returns the time client telling the present. */
      Beam::TimeClient get_time_client() const;

      /** Plays from the playhead. */
      void play();

      /** Pauses at the playhead. */
      void pause();

    private:
      struct Anchor {
        boost::posix_time::ptime m_time;
        boost::posix_time::time_duration m_playhead;
        Decimal m_speed;
      };
      Beam::TimeClient m_time_client;
      std::shared_ptr<TimelineModel> m_timeline;
      std::shared_ptr<DurationModel> m_playhead;
      std::shared_ptr<PlaybackSpeedModel> m_speed;
      std::shared_ptr<LocalPlaybackStateModel> m_state;
      std::shared_ptr<PlaybackStateModel> m_state_view;
      std::shared_ptr<LocalValueModel<boost::posix_time::ptime>> m_position;
      std::shared_ptr<ValueModel<boost::posix_time::ptime>> m_position_view;
      QTimer m_timer;
      Anchor m_anchor;
      boost::signals2::scoped_connection m_timeline_connection;
      boost::signals2::scoped_connection m_playhead_connection;
      boost::signals2::scoped_connection m_speed_connection;

      ReplaySession(const ReplaySession&) = delete;
      ReplaySession& operator =(const ReplaySession&) = delete;
      bool is_before_start();
      bool is_at_present();
      bool is_at_end() const;
      PlaybackState get_playing_state();
      void set_state(PlaybackState state);
      void reset_anchor();
      boost::posix_time::time_duration get_playhead_at(
        boost::posix_time::ptime now) const;
      void update_position();
      void update_state();
      void on_tick();
      void on_timeline(const Timeline& timeline);
      void on_playhead(const boost::posix_time::time_duration& playhead);
      void on_speed(const Decimal& speed);
  };

  /** A ValueModel over a ReplaySession. */
  using ReplaySessionModel = ValueModel<std::shared_ptr<ReplaySession>>;
}

#endif
