#ifndef SPIRE_DATETIMEINPUTDIALOG_HPP
#define SPIRE_DATETIMEINPUTDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <QDialog>
#include <QTimer>
#include "Spire/UI/UI.hpp"

class Ui_DateTimeInputDialog;

namespace Spire {

  /*! \class DateTimeInputDialog
      \brief Prompts the user for date/time.
   */
  class DateTimeInputDialog : public QDialog {
    public:

      //! Constructs a DateTimeInputDialog.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      DateTimeInputDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a DateTimeInputDialog with an initial date/time.
      /*!
        \param userProfile The user's profile.
        \param initialValue The initial date/time to display.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      DateTimeInputDialog(const boost::posix_time::ptime& initialValue,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~DateTimeInputDialog();

      //! Returns the date/time value represented by this dialog.
      boost::posix_time::ptime GetDateTime() const;

    private:
      std::unique_ptr<Ui_DateTimeInputDialog> m_ui;
      QTimer m_currentTimer;
      UserProfile* m_userProfile;

      void OnCurrentTimer();
  };
}

#endif
