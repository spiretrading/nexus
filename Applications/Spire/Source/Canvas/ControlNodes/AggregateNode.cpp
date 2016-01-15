#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

AggregateNode::AggregateNode() {
  AddChild("i0", make_unique<NoneNode>(TaskType::GetInstance()));
  SetType(TaskType::GetInstance());
  SetText("Aggregate");
}

AggregateNode::AggregateNode(vector<unique_ptr<CanvasNode>> nodes) {
  int index = 0;
  for(auto& node : nodes) {
    if(dynamic_cast<const NoneNode*>(node.get()) == nullptr) {
      AddChild("i" + boost::lexical_cast<std::string>(index), std::move(node));
    }
    ++index;
  }
  AddChild("i" + boost::lexical_cast<std::string>(GetChildren().size()),
    make_unique<NoneNode>(TaskType::GetInstance()));
  SetType(TaskType::GetInstance());
  SetText("Aggregate");
}

unique_ptr<CanvasNode> AggregateNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  if(dynamic_cast<const ReferenceNode*>(replacement.get())) {
    BOOST_THROW_EXCEPTION(CanvasOperationException("Reference not allowed."));
  }
  if(dynamic_cast<const NoneNode*>(replacement.get())) {
    if(&child == &GetChildren().back()) {
      return CanvasNode::Clone(*this);
    }
    auto clone = CanvasNode::Clone(*this);
    clone->RemoveChild(child);
    auto index = 0;
    for(auto& selfChild : clone->GetChildren()) {
      clone->RenameChild(selfChild,
        "i" + boost::lexical_cast<std::string>(index));
      ++index;
    }
    return std::move(clone);
  } else if(&child == &GetChildren().back()) {
    auto clone = CanvasNode::Clone(*this);
    if(clone->GetChildren().size() == 1) {
      clone->SetType(replacement->GetType());
    } else {
      replacement = Spire::Convert(std::move(replacement), clone->GetType());
    }
    clone->SetChild(child, std::move(replacement));
    clone->AddChild("i" + lexical_cast<string>(clone->GetChildren().size()),
      make_unique<NoneNode>(clone->GetType()));
    return std::move(clone);
  }
  return CanvasNode::Replace(child, std::move(replacement));
}

void AggregateNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> AggregateNode::Clone() const {
  return make_unique<AggregateNode>(*this);
}

AggregateNode::AggregateNode(ReceiveBuilder) {}
