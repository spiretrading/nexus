#ifndef SPIRE_REPLAY_WINDOW_HPP
#define SPIRE_REPLAY_WINDOW_HPP
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Spire/Playback/Playback.hpp"
#include "Spire/Playback/PlaybackSpeedBox.hpp"
#include "Spire/Playback/SeekBar.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /**
   * Displays a window that the user can control replay.
   */
  class ReplayWindow : public Window {
    public:

      /** Enumerates the different play states. */
      enum class State {

        /** The replay is currently paused. */
        PAUSED,

        /** The replay is currently playing. */
        REPLAYING,

        /** The replay is displaying data in real time. */
        REAL_TIME
      };

      /**
       * Signals that the state has changed.
       * @param state The changed play state.
       */
      using StateSignal = Signal<void (State state)>;

      /**
       * Constructs a ReplayWindow.
       * @param timeline The timeline that defines the start and duration of
       *        the playback.
       * @param time_client The time client that provides the current time.
       * @param playhead The current position within the timeline.
       * @param speed The playback speed.
       * @param min_date The minimum date for the replay.
       * @param parent The parent widget.
       */
      ReplayWindow(std::shared_ptr<TimelineModel> timeline,
        Beam::TimeService::TimeClientBox time_client,
        std::shared_ptr<DurationModel> playhead,
        std::shared_ptr<PlaybackSpeedModel> speed,
        boost::optional<boost::gregorian::date> min_date,
        QWidget* parent = nullptr);

      /** Returns the replay timeline. */
      const std::shared_ptr<TimelineModel>& get_timeline() const;

      /** Returns the time client. */
      Beam::TimeService::TimeClientBox get_time_client() const;

      /** Returns the timeline position. */
      const std::shared_ptr<DurationModel>& get_playhead() const;

      /** Returns the replay speed. */
      const std::shared_ptr<PlaybackSpeedModel>& get_speed() const;

      /** Connects a slot to the play state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      struct PlayheadModel;
      std::shared_ptr<PlayheadModel> m_playhead;
      LocalValueModel<State> m_state;
      SeekBar* m_seek_bar;
      Button* m_start_button;
      Button* m_end_button;
      ToggleButton* m_play_toggle;
      PlaybackSpeedBox* m_speed_box;
      boost::signals2::scoped_connection m_date_connection;
      boost::signals2::scoped_connection m_playhead_connection;
      boost::signals2::scoped_connection m_timeline_connection;

      bool is_playhead_at_end() const;
      void play();
      void pause();
      void on_play_checked(bool checked);
      void on_jump_to_start_click();
      void on_jump_to_end_click();
      void on_date_submit(const boost::optional<boost::gregorian::date>& date);
      void on_playhead_update(const boost::posix_time::time_duration& playhead);
      void on_state_update(const State& state);
      void on_timeline_update(const Timeline& timeline);
  };
}

#endif
