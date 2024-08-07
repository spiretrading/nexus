#ifndef SPIRE_SIGN_IN_UI_TESTER_HPP
#define SPIRE_SIGN_IN_UI_TESTER_HPP
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include "Spire/SignIn/SignIn.hpp"
#include "Spire/SignIn/Track.hpp"

namespace Spire {

  /**
   * Displays a window to simulate account sign in, rejection, and server
   * unavailable status.
   */
  class SignInUiTester : public QWidget {
    public:

      /** Constructs the SignInUiTester. */
      SignInUiTester();

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      bool m_is_closing;
      std::shared_ptr<LocalTrackModel> m_track;
      QCheckBox* m_classic_check_box;
      QCheckBox* m_preview_check_box;
      QCheckBox* m_live_trading_check_box;
      QCheckBox* m_staging_check_box;
      QCheckBox* m_uat_environment_check_box;
      QCheckBox* m_local_environment_check_box;
      QPushButton* m_accept_button;
      QPushButton* m_reject_button;
      QPushButton* m_error_button;
      QTextEdit* m_signals_text;
      QPushButton* m_rebuild_button;
      SignInWindow* m_window;

      void on_accept();
      void on_reject();
      void on_error();
      void on_rebuild();
      void on_sign_in(const std::string& username, const std::string& password,
        Track track, const std::string& server);
      void on_cancel();
  };
}

#endif
