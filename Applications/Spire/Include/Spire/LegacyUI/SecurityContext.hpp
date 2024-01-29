#ifndef SPIRE_SECURITYCONTEXT_HPP
#define SPIRE_SECURITYCONTEXT_HPP
#include <set>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {
namespace LegacyUI {

  /*! \class SecurityContext
      \brief Interface for a window that displays a Security.
   */
  class SecurityContext {
    public:

      //! Signals the displayed Security was changed.
      /*!
        \param security The Security being displayed.
      */
      typedef boost::signals2::signal<void (const Nexus::Security& security)>
        SecurityDisplaySignal;

      //! Finds the SecurityContext with the specified identifier.
      /*!
        \param identifier The identifier to find.
        \return The SecurityContext with the specified <i>identifier</i>.
      */
      static boost::optional<SecurityContext&> FindSecurityContext(
        const std::string& identifier);

      virtual ~SecurityContext();

      //! Links this SecurityContext to another.
      /*!
        \param context The SecurityContext to link to.
      */
      void Link(SecurityContext& context);

      //! Unlinks this SecurityContext from another.
      void Unlink();

      //! The Security currently displayed.
      const Nexus::Security& GetDisplayedSecurity() const;

      //! Returns an identifier unique to this SecurityContext.
      const std::string& GetIdentifier() const;

      //! Returns the identifier that this SecurityContext is linked to.
      const std::string& GetLinkedIdentifier() const;

      //! Connects a slot to the SecurityDisplaySignal.
      boost::signals2::connection ConnectSecurityDisplaySignal(
        const SecurityDisplaySignal::slot_type& slot) const;

    protected:

      //! Constructs a SecurityContext.
      SecurityContext();

      //! Constructs a SecurityContext with an existing identifier.
      /*!
        \param identifier The identifier.
      */
      SecurityContext(const std::string& identifier);

      //! Handles a link action from this SecurityContext to another.
      /*!
        \param context The SecurityContext to link to.
      */
      virtual void HandleLink(SecurityContext& context);

      //! Handles an unlink action of this SecurityContext from another.
      virtual void HandleUnlink();

      //! Sets the Security being displayed.
      /*!
        \param security The Security being displayed.
      */
      void SetDisplayedSecurity(const Nexus::Security& security);

    private:
      Nexus::Security m_security;
      std::string m_identifier;
      std::set<SecurityContext*> m_incomingLinks;
      SecurityContext* m_outgoingLink;
      mutable SecurityDisplaySignal m_securityDisplaySignal;
  };
}
}

#endif
