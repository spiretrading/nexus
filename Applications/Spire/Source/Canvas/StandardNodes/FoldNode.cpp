#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

FoldNode::FoldNode() {
  DefineFunction("Fold", { "combiner", "source" },
    MakeSignatures<FoldSignatures>());
}

boost::optional<const CanvasNode&> FoldNode::FindLeftOperand() const {
  auto combiner = FindChild("combiner");
  if(!combiner.is_initialized()) {
    return none;
  }
  for(auto& child : combiner->GetChildren()) {
    if(child.IsVisible() && !child.IsReadOnly()) {
      return child;
    }
  }
  return none;
}

boost::optional<const CanvasNode&> FoldNode::FindRightOperand() const {
  auto combiner = FindChild("combiner");
  if(!combiner.is_initialized()) {
    return none;
  }
  auto parameterCount = 0;
  for(auto& child : combiner->GetChildren()) {
    if(child.IsVisible() && !child.IsReadOnly()) {
      ++parameterCount;
      if(parameterCount == 2) {
        return child;
      }
    }
  }
  return none;
}

void FoldNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FoldNode::Clone() const {
  return std::make_unique<FoldNode>(*this);
}

FoldNode::FoldNode(ReceiveBuilder) {}
