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
using namespace std;

namespace {
  string GetReferentText(const string& referent) {
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

ReferenceNode::ReferenceNode(const string& referent)
    : m_referent(referent) {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(const string& referent, const CanvasType& type)
    : m_referent(referent) {
  SetType(type);
  SetText(GetReferentText(m_referent));
}

const string& ReferenceNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> ReferenceNode::SetReferent(
    const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = referent;
  clone->SetText(GetReferentText(referent));
  return std::move(clone);
}

unique_ptr<CanvasNode> ReferenceNode::Convert(const CanvasType& type) const {
  try {
    auto conversion = CanvasNode::Convert(type);
    return conversion;
  } catch(CanvasOperationException&) {
    auto clone = CanvasNode::Clone(*this);
    clone->SetType(type);
    return std::move(clone);
  }
}

void ReferenceNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> ReferenceNode::Clone() const {
  return make_unique<ReferenceNode>(*this);
}

unique_ptr<CanvasNode> ReferenceNode::Reset() const {
  return unique_ptr<ReferenceNode>();
}

ReferenceNode::ReferenceNode(ReceiveBuilder) {}

boost::optional<const CanvasNode&> Spire::FindAnchor(const CanvasNode& node) {
  auto i = &node;
  unordered_set<const CanvasNode*> visitedNodes;
  while(dynamic_cast<const ReferenceNode*>(i)) {
    if(!visitedNodes.insert(i).second) {
      return none;
    }
    auto referent = dynamic_cast<const ReferenceNode*>(i)->FindReferent();
    if(!referent.is_initialized()) {
      return none;
    }
    i = &*referent;
  }
  return *i;
}
