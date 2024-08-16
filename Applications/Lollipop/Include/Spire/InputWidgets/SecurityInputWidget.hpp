#ifndef SPIRE_SECURITY_INPUT_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_WIDGET_HPP
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"

class QLineEdit;

namespace Spire {

  /** Displays a widget to input and display a Security. */
  class SecurityInputWidget : public QWidget {
    public:

      /**
       * Signals a change to the Security.
       * @param value The updated Security.
       */
      using SecurityUpdatedSignal =
        boost::signals2::signal<void (const Nexus::Security& value)>;

      /**
       * Constructs a SecurityInputWidget.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit SecurityInputWidget(
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      /**
       * Constructs a SecurityInputWidget.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit SecurityInputWidget(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      /**
       * Initializes this widget.
       * @param userProfile The user's profile.
       */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      /** Returns the Security displayed. */
      const Nexus::Security& GetSecurity() const;

      /**
       * Sets the Security to display.
       * @param security The Security to display.
       */
      void SetSecurity(Nexus::Security security);

      /**
       * Makes this widget read-only.
       * @param value <code>true</code> to make this widget read-only.
       */
      void SetReadOnly(bool value);

      /**
       * Connects a slot to the SecurityUpdatedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectSecurityUpdatedSignal(
        const SecurityUpdatedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseDoubleClickEvent(QMouseEvent* event) override;

    private:
      UserProfile* m_userProfile;
      Nexus::Security m_security;
      QLineEdit* m_lineEdit;
      bool m_isReadOnly;
      mutable SecurityUpdatedSignal m_securityUpdatedSignal;
  };
}

#endif
