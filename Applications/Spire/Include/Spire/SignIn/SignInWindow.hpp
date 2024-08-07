#ifndef SPIRE_SIGN_IN_WINDOW_HPP
#define SPIRE_SIGN_IN_WINDOW_HPP
#include <string>
#include <QPoint>
#include <QWidget>
#include "Spire/SignIn/SignIn.hpp"
#include "Spire/SignIn/Track.hpp"
#include "Spire/Ui/KeyObserver.hpp"

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

        /** The user requested to cancel. */
        CANCELLING,

        /** An error occured. */
        ERROR
      };

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

      /**
       * Constructs a SignInWindow in the NONE state.
       * @param version The application build version.
       * @param tracks The list of tracks the user can run.
       * @param track The currently selected track among the provided
       *        <i>tracks</i>.
       * @param servers The list of servers available to sign in to.
       */
      SignInWindow(std::string version, std::vector<Track> tracks,
        std::shared_ptr<TrackModel> track, std::vector<std::string> servers,
        QWidget* parent = nullptr);

      /** Sets the state to display to the user. */
      void set_state(State state);

      /** Sets an error state. */
      void set_error(const QString& message);

      /** Connects a slot to the SignInSignal. */
      boost::signals2::connection connect_sign_in_signal(
        const SignInSignal::slot_type& slot) const;

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
      State m_state;
      DropShadow* m_shadow;
      TrackMenuButton* m_track_button;
      TextBox* m_status_label;
      TextBox* m_username_text_box;
      boost::optional<KeyObserver> m_username_key_observer;
      TextBox* m_password_text_box;
      boost::optional<KeyObserver> m_password_key_observer;
      DropDownBox* m_server_box;
      Button* m_sign_in_button;
      QWidget* m_last_focus;
      bool m_is_dragging;
      QPoint m_last_pos;
      ChromaHashWidget* m_chroma_hash_widget;

      void reset_all();
      void reset_visuals();
      void try_sign_in();
      void on_key_press(QWidget& target, const QKeyEvent& event);
  };
}

#endif
