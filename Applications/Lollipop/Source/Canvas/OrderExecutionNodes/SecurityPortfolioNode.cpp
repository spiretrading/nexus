#include "Spire/Canvas/OrderExecutionNodes/SecurityPortfolioNode.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Spire;
using namespace std;

SecurityPortfolioNode::SecurityPortfolioNode() {
  SetText("Security Portfolio");
  SetType(GetPortfolioEntryRecordType());
  AddChild("source", make_unique<ReferenceNode>());
}

unique_ptr<CanvasNode> SecurityPortfolioNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(&child == &GetChildren().front()) {
    if(dynamic_cast<const ReferenceNode*>(replacement.get()) != nullptr) {
      return CanvasNode::Replace(child, std::move(replacement));
    }
    BOOST_THROW_EXCEPTION(CanvasOperationException("Must reference a task."));
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void SecurityPortfolioNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SecurityPortfolioNode::Clone() const {
  return make_unique<SecurityPortfolioNode>(*this);
}

SecurityPortfolioNode::SecurityPortfolioNode(ReceiveBuilder) {}
