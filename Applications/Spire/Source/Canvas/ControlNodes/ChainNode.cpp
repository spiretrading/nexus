#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

ChainNode::ChainNode() {
  AddChild("i0", make_unique<NoneNode>(UnionType::GetAnyValueType()));
  SetType(UnionType::GetAnyValueType());
  SetText("Chain");
}

ChainNode::ChainNode(vector<unique_ptr<CanvasNode>> nodes) {
  auto index = 0;
  for(auto& node : nodes) {
    if(dynamic_cast<const NoneNode*>(node.get()) == nullptr) {
      AddChild("i" + boost::lexical_cast<std::string>(index), std::move(node));
    }
    ++index;
  }
  std::shared_ptr<CanvasType> type;
  if(nodes.empty()) {
    type = UnionType::GetAnyValueType();
  } else {
    type = GetChildren().front().GetType();
  }
  AddChild("i" + boost::lexical_cast<std::string>(GetChildren().size()),
    make_unique<NoneNode>(*type));
  SetType(*type);
  SetText("Chain");
}

unique_ptr<CanvasNode> ChainNode::Convert(const CanvasType& type) const {
  auto clone = CanvasNode::Clone(*this);
  for(auto& child : clone->GetChildren()) {
    clone->SetChild(child, child.Convert(type));
  }
  clone->SetType(type);
  return std::move(clone);
}

unique_ptr<CanvasNode> ChainNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
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

void ChainNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> ChainNode::Clone() const {
  return make_unique<ChainNode>(*this);
}

ChainNode::ChainNode(ReceiveBuilder) {}
