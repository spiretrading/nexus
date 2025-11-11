#ifndef SPIRE_LINKSECURITYCONTEXTACTION_HPP
#define SPIRE_LINKSECURITYCONTEXTACTION_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QAction>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class LinkSecurityContextAction
      \brief An action to link to a SecurityContext.
   */
  class LinkSecurityContextAction : public QAction {
    public:

      //! Returns a list of LinkSecurityContextActions based on all currently
      //! open SecurityContexts.
      /*!
        \param context The SecurityContext that will be linked.
        \param currentLinkIdentifier The identifier of the SecurityContext that
               <i>context<i> is currently linked to.
        \param parent The parent of the actions to be built.
      */
      static std::vector<std::unique_ptr<LinkSecurityContextAction>>
        MakeActions(SecurityContext* context,
        const std::string& currentLinkIdentifier, QObject* parent,
        const UserProfile& userProfile);

      //! Constructs a LinkSecurityContextAction.
      /*!
        \param securityContext The SecurityContext to link to.
      */
      LinkSecurityContextAction(
        Beam::Ref<SecurityContext> securityContext, QObject* parent);

      virtual ~LinkSecurityContextAction();

      //! Returns the SecurityContext to link to.
      SecurityContext& GetSecurityContext();

      //! Executes the link action.
      /*!
        \param context The SecurityContext to execute the link action on.
      */
      void Execute(Beam::Out<SecurityContext> context);

    private:
      SecurityContext* m_securityContext;
  };
}
}

#endif
