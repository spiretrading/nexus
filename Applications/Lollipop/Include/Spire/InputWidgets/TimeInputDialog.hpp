#ifndef SPIRE_TIMEINPUTDIALOG_HPP
#define SPIRE_TIMEINPUTDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <QDialog>
#include <QTimer>
#include "Spire/Spire/Spire.hpp"

class Ui_TimeInputDialog;

namespace Spire {

  /*! \class TimeInputDialog
      \brief Prompts the user to enter a time of day.
   */
  class TimeInputDialog : public QDialog {
    public:

      //! Constructs a TimeInputDialog.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeInputDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a TimeInputDialog with an initial date/time.
      /*!
        \param initialValue The initial time to display.
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeInputDialog(const boost::posix_time::time_duration& initialValue,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TimeInputDialog();

      //! Returns the time value represented by this dialog.
      boost::posix_time::time_duration GetTime() const;

    private:
      std::unique_ptr<Ui_TimeInputDialog> m_ui;
      QTimer m_currentTimer;
      UserProfile* m_userProfile;

      void OnCurrentTimer();
  };
}

#endif
