#include "Spire/LegacyUI/LinkTickerContextAction.hpp"
#include <Beam/Utilities/ToString.hpp>
#include <QApplication>
#include <QWidget>
#include "Spire/LegacyUI/TickerContext.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

vector<unique_ptr<LinkTickerContextAction>>
    LinkTickerContextAction::MakeActions(TickerContext* context,
    const string& currentLinkIdentifier, QObject* parent,
    const UserProfile& userProfile) {
  vector<unique_ptr<LinkTickerContextAction>> linkActions;
  QWidgetList widgets = QApplication::topLevelWidgets();
  for(QWidget* widget : widgets) {
    TickerContext* tickerContext = dynamic_cast<TickerContext*>(widget);
    if(widget->isVisible() && tickerContext != nullptr &&
        tickerContext != context) {
      unique_ptr<LinkTickerContextAction> linkAction =
        std::make_unique<LinkTickerContextAction>(Ref(*tickerContext),
        parent);
      if(currentLinkIdentifier == tickerContext->GetIdentifier()) {
        linkAction->setCheckable(true);
        linkAction->setChecked(true);
      }
      linkActions.push_back(std::move(linkAction));
    }
  }
  std::sort(linkActions.begin(), linkActions.end(),
    [&] (const unique_ptr<LinkTickerContextAction>& lhs,
        const unique_ptr<LinkTickerContextAction>& rhs) -> bool {
      return to_string(lhs->GetTickerContext().GetDisplayedTicker()) <
        to_string(rhs->GetTickerContext().GetDisplayedTicker());
    });
  return linkActions;
}

LinkTickerContextAction::LinkTickerContextAction(
    Ref<TickerContext> tickerContext, QObject* parent)
    : QAction(dynamic_cast<QWidget*>(tickerContext.get())->windowTitle(),
        parent),
      m_tickerContext(tickerContext.get()) {}

LinkTickerContextAction::~LinkTickerContextAction() {}

TickerContext& LinkTickerContextAction::GetTickerContext() {
  return *m_tickerContext;
}

void LinkTickerContextAction::Execute(Out<TickerContext> context) {
  if(m_tickerContext->GetIdentifier() == context->GetLinkedIdentifier()) {
    context->Unlink();
  } else {
    context->Link(*m_tickerContext);
    if(m_tickerContext->GetLinkedIdentifier().empty()) {
      m_tickerContext->Link(*context);
    }
  }
}
