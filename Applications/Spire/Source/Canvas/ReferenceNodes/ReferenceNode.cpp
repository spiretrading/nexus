#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include <unordered_set>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;

namespace {
  std::string GetReferentText(const std::string& referent) {
    if(referent.empty()) {
      return "Reference";
    }
    return "@" + referent;
  }
}

ReferenceNode::ReferenceNode() {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(const std::string& referent)
    : m_referent(referent) {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(
    const std::string& referent, const CanvasType& type)
    : m_referent(referent) {
  SetType(type);
  SetText(GetReferentText(m_referent));
}

const std::string& ReferenceNode::GetReferent() const {
  return m_referent;
}

std::unique_ptr<CanvasNode> ReferenceNode::SetReferent(
    const std::string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  clone->SetText(GetReferentText(referent));
  return clone;
}

std::unique_ptr<CanvasNode>
    ReferenceNode::Convert(const CanvasType& type) const {
  try {
    return CanvasNode::Convert(type);
  } catch(const CanvasOperationException&) {
    auto clone = CanvasNode::Clone(*this);
    clone->SetType(type);
    return clone;
  }
}

void ReferenceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> ReferenceNode::Clone() const {
  return std::make_unique<ReferenceNode>(*this);
}

std::unique_ptr<CanvasNode> ReferenceNode::Reset() const {
  return std::unique_ptr<ReferenceNode>();
}

ReferenceNode::ReferenceNode(ReceiveBuilder) {}

optional<const CanvasNode&> Spire::FindAnchor(const CanvasNode& node) {
  auto i = &node;
  auto visitedNodes = std::unordered_set<const CanvasNode*>();
  while(dynamic_cast<const ReferenceNode*>(i)) {
    if(!visitedNodes.insert(i).second) {
      return none;
    }
    auto referent = dynamic_cast<const ReferenceNode*>(i)->FindReferent();
    if(!referent) {
      return none;
    }
    i = &*referent;
  }
  return *i;
}
