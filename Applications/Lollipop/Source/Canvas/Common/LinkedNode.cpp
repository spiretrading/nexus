#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

boost::optional<const CanvasNode&> LinkedNode::FindReferent() const {
  if(GetReferent().empty()) {
    return none;
  }
  auto name = GetReferent();
  boost::optional<const CanvasNode&> node =
    dynamic_cast<const CanvasNode&>(*this);
  while(node.is_initialized() && !name.empty() && name[0] == '<') {
    if(IsRoot(*node)) {
      return none;
    }
    node = node->GetParent();
    if(node.is_initialized() &&
        dynamic_cast<const BlotterTaskMonitorNode*>(&*node) && IsRoot(*node)) {
      return node;
    }
    name = name.substr(1);
  }
  if(!node.is_initialized()) {
    return none;
  }
  if(name.empty()) {
    if(dynamic_cast<const BlotterTaskMonitorNode*>(&*node)) {
      return node->GetParent()->GetChildren().front();
    }
    return node;
  }
  if(IsRoot(*node)) {
    return none;
  }
  return node->GetParent()->FindNode(name);
}

int Spire::GetReferentHeight(const LinkedNode& node) {
  const auto& name = node.GetReferent();
  int height = 0;
  for(char i : node.GetReferent()) {
    if(i == '<') {
      ++height;
    } else {
      break;
    }
  }
  return height;
}

unique_ptr<CanvasNode> Spire::Relink(const CanvasNode& root) {
  CanvasNodeBuilder builder(root);
  for(const auto& node : BreadthFirstView(root)) {
    if(auto linkedNode = dynamic_cast<const LinkedNode*>(&node)) {
      auto depth = GetReferentHeight(*linkedNode);
      if(depth > GetHeight(root, node)) {
        builder.Replace(node,
          linkedNode->SetReferent("<" + linkedNode->GetReferent()));
      }
    }
  }
  return builder.Make();
}
