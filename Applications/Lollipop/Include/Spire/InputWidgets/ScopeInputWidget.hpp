#ifndef SPIRE_SCOPE_INPUT_WIDGET_HPP
#define SPIRE_SCOPE_INPUT_WIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/UI/UI.hpp"

class QLineEdit;

namespace Spire {

  /** Displays a widget to input and edit a Scope. */
  class ScopeInputWidget : public QWidget {
    public:

      /**
       * Signals a change to the value.
       * @param scope The updated Scope.
       */
      using UpdatedSignal =
        boost::signals2::signal<void (const Nexus::Scope& scope)>;

      /**
       * Constructs a ScopeInputWidget.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit ScopeInputWidget(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a ScopeInputWidget.
       * @param scope The Scope to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      ScopeInputWidget(Nexus::Scope scope, Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr);

      /** Returns the Scope. */
      const Nexus::Scope& GetScope() const;

      /** Sets the Scope. */
      void SetScope(const Nexus::Scope& scope);

      /**
       * Makes this widget read-only.
       * @param value <code>true</code> to make this widget read-only.
       */
      void SetReadOnly(bool value);

      /**
       * Connects a slot to the UpdatedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectUpdatedSignal(
        const UpdatedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseDoubleClickEvent(QMouseEvent* event) override;

    private:
      UserProfile* m_userProfile;
      Nexus::Scope m_scope;
      bool m_isReadOnly;
      QLineEdit* m_lineEdit;
      mutable UpdatedSignal m_updatedSignal;
  };
}

#endif
