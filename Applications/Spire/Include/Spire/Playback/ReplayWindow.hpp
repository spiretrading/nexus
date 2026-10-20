#ifndef SPIRE_REPLAY_WINDOW_HPP
#define SPIRE_REPLAY_WINDOW_HPP
#include "Spire/Playback/PlaybackSpeedBox.hpp"
#include "Spire/Playback/ReplayAttachMenuButton.hpp"
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/Playback/SeekBar.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
  class Button;
  class ToggleButton;

  /**
   * Displays a window that the user can control replay.
   */
  class ReplayWindow : public Window {
    public:

      /**
       * Constructs a ReplayWindow.
       * @param session The session that the window controls.
       * @param roster The members whose PropertyHubs form the attachment
       *        targets.
       * @param attachments The PropertyHubs that the replay is attached to,
       *        emptied when the window is closed.
       * @param min_date The minimum date for the replay.
       * @param parent The parent widget.
       */
      ReplayWindow(std::shared_ptr<ReplaySession> session,
        std::shared_ptr<ListModel<PropertyHubMember*>> roster,
        std::shared_ptr<PropertyHubListModel> attachments,
        boost::optional<boost::gregorian::date> min_date,
        QWidget* parent = nullptr);

      /** Returns the session that the window controls. */
      const std::shared_ptr<ReplaySession>& get_session() const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      std::shared_ptr<ReplaySession> m_session;
      std::shared_ptr<PropertyHubListModel> m_attachments;
      SeekBar* m_seek_bar;
      Button* m_start_button;
      Button* m_end_button;
      ToggleButton* m_play_toggle;
      PlaybackSpeedBox* m_speed_box;
      boost::signals2::scoped_connection m_date_connection;
      boost::signals2::scoped_connection m_playhead_connection;
      boost::signals2::scoped_connection m_state_connection;
      boost::signals2::scoped_connection m_timeline_connection;

      bool is_playhead_at_end() const;
      void on_play_checked(bool checked);
      void on_jump_to_start_click();
      void on_jump_to_end_click();
      void on_date_submit(const boost::optional<boost::gregorian::date>& date);
      void on_playhead_update(const boost::posix_time::time_duration& playhead);
      void on_state_update(PlaybackState state);
      void on_timeline_update(const Timeline& timeline);
  };
}

#endif
