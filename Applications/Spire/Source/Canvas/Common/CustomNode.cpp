#include "Spire/Canvas/Common/CustomNode.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;

CustomNode::Child::Child()
  : m_type(UnionType::GetEmptyType()) {}

CustomNode::Child::Child(std::string name, const CanvasType& type)
  : m_name(std::move(name)),
    m_type(type) {}

CustomNode::CustomNode(std::string name, std::vector<Child> children)
    : m_children(std::move(children)) {
  SetText(std::move(name));
  for(auto& child : m_children) {
    AddChild(child.m_name, MakeDefaultCanvasNode(*child.m_type));
  }
  SetType(*m_children.front().m_type);
}

const std::vector<CustomNode::Child>& CustomNode::GetChildrenDetails() const {
  return m_children;
}

optional<const CanvasNode&>
    CustomNode::FindNode(const std::string& name) const {
  if(auto node = CanvasNode::FindNode(name)) {
    return node;
  }
  return GetChildren().front().FindNode(name);
}

std::unique_ptr<CanvasNode> CustomNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto builder = CanvasNodeBuilder(*replacement);
  for(auto& selfChild : GetChildren()) {
    auto name = selfChild.GetName();
    if(name.find("\\.") != std::string::npos) {
      replace_all(name, "\\.", ".");
      auto suffix = SplitName(name);
      if(child.GetName() != suffix.m_identifier) {
        continue;
      }
      auto target = replacement->FindNode(suffix.m_suffix);
      if(!target) {
        continue;
      }
      auto original = [&] {
        auto targetProxy = dynamic_cast<const ProxyNode*>(&*target);
        if(targetProxy) {
          return &targetProxy->GetOriginal();
        } else {
          return &*target;
        }
      }();
      auto path = AppendCanvasNodePaths(GetPath(*target, *replacement),
        GetPath(child, selfChild));
      auto proxy = std::make_unique<ProxyNode>(
        path, selfChild.GetType(), CanvasNode::Clone(*original));
      builder.Replace(*target, std::move(proxy));
    }
  }
  return CanvasNode::Replace(child, builder.Make());
}

void CustomNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> CustomNode::Clone() const {
  return std::make_unique<CustomNode>(*this);
}

CustomNode::CustomNode(ReceiveBuilder) {}
