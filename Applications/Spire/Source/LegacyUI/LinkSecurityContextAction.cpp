#include "Spire/LegacyUI/LinkSecurityContextAction.hpp"
#include <QApplication>
#include <QWidget>
#include "Spire/LegacyUI/SecurityContext.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

vector<unique_ptr<LinkSecurityContextAction>>
    LinkSecurityContextAction::MakeActions(SecurityContext* context,
    const string& currentLinkIdentifier, QObject* parent,
    const UserProfile& userProfile) {
  vector<unique_ptr<LinkSecurityContextAction>> linkActions;
  QWidgetList widgets = QApplication::topLevelWidgets();
  for(QWidget* widget : widgets) {
    SecurityContext* securityContext = dynamic_cast<SecurityContext*>(widget);
    if(widget->isVisible() && securityContext != nullptr &&
        securityContext != context) {
      unique_ptr<LinkSecurityContextAction> linkAction =
        std::make_unique<LinkSecurityContextAction>(Ref(*securityContext),
        parent);
      if(currentLinkIdentifier == securityContext->GetIdentifier()) {
        linkAction->setCheckable(true);
        linkAction->setChecked(true);
      }
      linkActions.push_back(std::move(linkAction));
    }
  }
  std::sort(linkActions.begin(), linkActions.end(),
    [&] (const unique_ptr<LinkSecurityContextAction>& lhs,
        const unique_ptr<LinkSecurityContextAction>& rhs) -> bool {
      return ToString(lhs->GetSecurityContext().GetDisplayedSecurity(),
        userProfile.GetMarketDatabase()) <
        ToString(rhs->GetSecurityContext().GetDisplayedSecurity(),
        userProfile.GetMarketDatabase());
    });
  return linkActions;
}

LinkSecurityContextAction::LinkSecurityContextAction(
    Ref<SecurityContext> securityContext, QObject* parent)
    : QAction(dynamic_cast<QWidget*>(securityContext.Get())->windowTitle(),
        parent),
      m_securityContext(securityContext.Get()) {}

LinkSecurityContextAction::~LinkSecurityContextAction() {}

SecurityContext& LinkSecurityContextAction::GetSecurityContext() {
  return *m_securityContext;
}

void LinkSecurityContextAction::Execute(Out<SecurityContext> context) {
  if(m_securityContext->GetIdentifier() == context->GetLinkedIdentifier()) {
    context->Unlink();
  } else {
    context->Link(*m_securityContext);
    if(m_securityContext->GetLinkedIdentifier().empty()) {
      m_securityContext->Link(*context);
    }
  }
}
