#ifndef SPIRE_TIMEINPUTWIDGET_HPP
#define SPIRE_TIMEINPUTWIDGET_HPP
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/UI/UI.hpp"

class QLineEdit;

namespace Spire {

  /*! \class TimeInputWidget
      \brief Displays a widget to input and display a time of day.
   */
  class TimeInputWidget : public QWidget {
    public:

      //! Signals a change to the time.
      /*!
        \param value The updated time.
      */
      using TimeUpdatedSignal = boost::signals2::signal<
        void (const boost::posix_time::time_duration& value)>;

      //! Constructs a TimeInputWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeInputWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Constructs a TimeInputWidget.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeInputWidget(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      virtual ~TimeInputWidget();

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      //! Returns the time.
      const boost::posix_time::time_duration& GetTime() const;

      //! Sets the time to display.
      /*!
        \param time The time to display.
      */
      void SetTime(boost::posix_time::time_duration time);

      //! Makes this widget read-only.
      /*!
        \param value <code>true</code> to make this widget read-only.
      */
      void SetReadOnly(bool value);

      //! Connects a slot to the TimeUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectTimeUpdatedSignal(
        const TimeUpdatedSignal::slot_type& slot) const;

    protected:
      virtual bool eventFilter(QObject* receiver, QEvent* event);
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void mouseDoubleClickEvent(QMouseEvent* event);

    private:
      UserProfile* m_userProfile;
      boost::posix_time::time_duration m_time;
      QLineEdit* m_lineEdit;
      bool m_isReadOnly;
      mutable TimeUpdatedSignal m_timeUpdatedSignal;
  };
}

#endif
