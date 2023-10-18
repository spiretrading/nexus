#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include <boost/lexical_cast.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;

AggregateNode::AggregateNode() {
  AddChild("i0", std::make_unique<NoneNode>(UnionType::GetAnyType()));
  SetType(UnionType::GetAnyType());
  SetText("Aggregate");
}

AggregateNode::AggregateNode(std::vector<std::unique_ptr<CanvasNode>> nodes) {
  auto index = 0;
  for(auto& node : nodes) {
    if(!dynamic_cast<const NoneNode*>(node.get())) {
      AddChild("i" + boost::lexical_cast<std::string>(index), std::move(node));
    }
    ++index;
  }
  auto type = [&] {
    if(nodes.empty()) {
      return static_cast<const CanvasType*>(&UnionType::GetAnyType());
    } else {
      return &GetChildren().front().GetType();
    }
  }();
  AddChild("i" + boost::lexical_cast<std::string>(GetChildren().size()),
    std::make_unique<NoneNode>(*type));
  SetType(*type);
  SetText("Aggregate");
}

std::unique_ptr<CanvasNode>
    AggregateNode::Convert(const CanvasType& type) const {
  auto clone = CanvasNode::Clone(*this);
  for(auto& child : clone->GetChildren()) {
    clone->SetChild(child, child.Convert(type));
  }
  clone->SetType(type);
  return clone;
}

std::unique_ptr<CanvasNode> AggregateNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  if(dynamic_cast<const NoneNode*>(replacement.get())) {
    if(&child == &GetChildren().back()) {
      return CanvasNode::Clone(*this);
    }
    auto clone = CanvasNode::Clone(*this);
    clone->RemoveChild(child);
    auto index = 0;
    for(auto& selfChild : clone->GetChildren()) {
      clone->RenameChild(selfChild, "i" + lexical_cast<std::string>(index));
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
    clone->AddChild(
      "i" + lexical_cast<std::string>(clone->GetChildren().size()),
      std::make_unique<NoneNode>(clone->GetType()));
    return std::move(clone);
  }
  auto type = std::shared_ptr<CanvasType>(replacement->GetType());
  return CanvasNode::Replace(child, std::move(replacement))->Convert(*type);
}

void AggregateNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> AggregateNode::Clone() const {
  return std::make_unique<AggregateNode>(*this);
}

AggregateNode::AggregateNode(ReceiveBuilder) {}
