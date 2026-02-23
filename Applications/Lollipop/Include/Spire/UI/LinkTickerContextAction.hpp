#ifndef SPIRE_LINK_TICKER_CONTEXT_ACTION_HPP
#define SPIRE_LINK_TICKER_CONTEXT_ACTION_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QAction>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class LinkTickerContextAction
      \brief An action to link to a TickerContext.
   */
  class LinkTickerContextAction : public QAction {
    public:

      //! Returns a list of LinkTickerContextActions based on all currently
      //! open TickerContexts.
      /*!
        \param context The TickerContext that will be linked.
        \param currentLinkIdentifier The identifier of the TickerContext that
               <i>context<i> is currently linked to.
        \param parent The parent of the actions to be built.
      */
      static std::vector<std::unique_ptr<LinkTickerContextAction>>
        MakeActions(TickerContext* context,
        const std::string& currentLinkIdentifier, QObject* parent,
        const UserProfile& userProfile);

      //! Constructs a LinkTickerContextAction.
      /*!
        \param tickerContext The TickerContext to link to.
      */
      LinkTickerContextAction(
        Beam::Ref<TickerContext> tickerContext, QObject* parent);

      virtual ~LinkTickerContextAction();

      //! Returns the TickerContext to link to.
      TickerContext& GetTickerContext();

      //! Executes the link action.
      /*!
        \param context The TickerContext to execute the link action on.
      */
      void Execute(Beam::Out<TickerContext> context);

    private:
      TickerContext* m_tickerContext;
  };
}
}

#endif
