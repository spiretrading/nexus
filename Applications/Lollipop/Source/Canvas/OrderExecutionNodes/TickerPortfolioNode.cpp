#include "Spire/Canvas/OrderExecutionNodes/TickerPortfolioNode.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

TickerPortfolioNode::TickerPortfolioNode() {
  SetText("Ticker Portfolio");
  SetType(GetPortfolioEntryRecordType());
  AddChild("source", std::make_unique<ReferenceNode>());
}

unique_ptr<CanvasNode> TickerPortfolioNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(&child == &GetChildren().front()) {
    if(dynamic_cast<const ReferenceNode*>(replacement.get()) != nullptr) {
      return CanvasNode::Replace(child, std::move(replacement));
    }
    BOOST_THROW_EXCEPTION(CanvasOperationException("Must reference a task."));
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void TickerPortfolioNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TickerPortfolioNode::Clone() const {
  return std::make_unique<TickerPortfolioNode>(*this);
}
