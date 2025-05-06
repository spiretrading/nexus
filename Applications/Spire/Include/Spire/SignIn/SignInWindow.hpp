#ifndef SPIRE_SIGN_IN_WINDOW_HPP
#define SPIRE_SIGN_IN_WINDOW_HPP
#include <string>
#include <QPoint>
#include <QWidget>
#include "Spire/SignIn/SignIn.hpp"
#include "Spire/SignIn/SignInUpdateBox.hpp"
#include "Spire/SignIn/Track.hpp"
#include "Spire/Ui/KeyObserver.hpp"
#include "Spire/Ui/TextBox.hpp"

#ifdef ERROR
  #undef ERROR
#endif

namespace Spire {

  /** Displays the sign in window. */
  class SignInWindow : public QWidget {
    public:

      /** The sign in state to display to the user. */
      enum class State {

        /** Nothing to report. */
        NONE,

        /** The user is in the process of signing in. */
        SIGNING_IN,

        /** The application is being updated. */
        UPDATING,

        /** The user requested to cancel. */
        CANCELLING,

        /** An error occured. */
        ERROR
      };

      using Operation = SignInUpdateBox::Operation;

      /**
       * Signals an attempt to sign in.
       * @param username The username to sign in with.
       * @param password The password to sign in with.
       * @param track The track to run.
       * @param server The server to sign in to.
       */
      using SignInSignal =
        Signal<void (const std::string& username, const std::string& password,
          Track track, const std::string& server)>;

      /** Signals to cancel a previous sign in operation. */
      using CancelSignal = Signal<void ()>;

      using RetrySignal = SignInUpdateBox::RetrySignal;

      /**
       * Constructs a SignInWindow in the NONE state.
       * @param version The application build version.
       * @param tracks The list of tracks the user can run.
       * @param track The currently selected track among the provided
       *        <i>tracks</i>.
       * @param servers The list of servers available to sign in to.
       * @param download_progress The amount of progress downloading the update.
       * @param installation_progress The amount of progress performing the
       *        installation.
       * @param time_left The amount of time left to perform the current
       *        operation.
       */
      SignInWindow(std::string version, std::vector<Track> tracks,
        std::shared_ptr<TrackModel> track, std::vector<std::string> servers,
        std::shared_ptr<ProgressModel> download_progress,
        std::shared_ptr<ProgressModel> installation_progress,
        std::shared_ptr<ValueModel<boost::posix_time::time_duration>> time_left,
        QWidget* parent = nullptr);

      /** Returns the state. */
      State get_state() const;

      /** Sets the state to display to the user. */
      void set_state(State state);

      /** Sets an error state. */
      void set_error(const QString& message);

      /** Connects a slot to the SignInSignal. */
      boost::signals2::connection connect_sign_in_signal(
        const SignInSignal::slot_type& slot) const;

      /** Connects a slot to the retry signal. */
      boost::signals2::connection connect_retry_signal(
        const RetrySignal::slot_type& slot) const;

      /** Connects a slot to the cancel signal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable SignInSignal m_sign_in_signal;
      mutable CancelSignal m_cancel_signal;
      mutable RetrySignal m_retry_signal;
      std::string m_version;
      std::vector<std::string> m_servers;
      std::shared_ptr<ProgressModel> m_download_progress;
      std::shared_ptr<ProgressModel> m_installation_progress;
      std::shared_ptr<ValueModel<boost::posix_time::time_duration>> m_time_left;
      State m_state;
      DropShadow* m_shadow;
      TrackMenuButton* m_track_button;
      TextBox* m_status_label;
      std::shared_ptr<TextModel> m_username;
      TextBox* m_username_text_box;
      boost::optional<KeyObserver> m_username_key_observer;
      std::shared_ptr<TextModel> m_password;
      TextBox* m_password_text_box;
      boost::optional<KeyObserver> m_password_key_observer;
      DropDownBox* m_server_box;
      Button* m_sign_in_button;
      SignInUpdateBox* m_update_box;
      QWidget* m_last_focus;
      bool m_is_dragging;
      QPoint m_last_pos;
      ChromaHashWidget* m_chroma_hash_widget;

      void layout_sign_in();
      void clear_sign_in();
      void layout_update();
      void clear_update();
      void reset_all();
      void reset_visuals();
      void try_sign_in();
      void on_key_press(QWidget& target, const QKeyEvent& event);
      void on_cancel_update();
  };
}

#endif
