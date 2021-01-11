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
using namespace std;

CustomNode::Child::Child()
    : m_type(UnionType::GetEmptyType()) {}

CustomNode::Child::Child(string name, const CanvasType& type)
    : m_name(std::move(name)),
      m_type(type) {}

CustomNode::CustomNode(string name, vector<Child> children)
    : m_children(std::move(children)) {
  SetText(std::move(name));
  for(const auto& child : m_children) {
    AddChild(child.m_name, MakeDefaultCanvasNode(*child.m_type));
  }
  SetType(*m_children.front().m_type);
}

const vector<CustomNode::Child>& CustomNode::GetChildrenDetails() const {
  return m_children;
}

boost::optional<const CanvasNode&> CustomNode::FindNode(
    const string& name) const {
  auto node = CanvasNode::FindNode(name);
  if(node.is_initialized()) {
    return node;
  }
  return GetChildren().front().FindNode(name);
}

unique_ptr<CanvasNode> CustomNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  CanvasNodeBuilder builder(*replacement);
  for(const auto& selfChild : GetChildren()) {
    auto name = selfChild.GetName();
    if(name.find("\\.") != string::npos) {
      replace_all(name, "\\.", ".");
      auto suffix = SplitName(name);
      if(child.GetName() != suffix.m_identifier) {
        continue;
      }
      auto target = replacement->FindNode(suffix.m_suffix);
      if(!target.is_initialized()) {
        continue;
      }
      const CanvasNode* original;
      auto targetProxy = dynamic_cast<const ProxyNode*>(&*target);
      if(targetProxy == nullptr) {
        original = &*target;
      } else {
        original = &targetProxy->GetOriginal();
      }
      auto path = AppendCanvasNodePaths(GetPath(*target, *replacement),
        GetPath(child, selfChild));
      auto proxy = make_unique<ProxyNode>(path, selfChild.GetType(),
        CanvasNode::Clone(*original));
      builder.Replace(*target, std::move(proxy));
    }
  }
  return CanvasNode::Replace(child, builder.Make());
}

void CustomNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> CustomNode::Clone() const {
  return make_unique<CustomNode>(*this);
}

CustomNode::CustomNode(ReceiveBuilder) {}
