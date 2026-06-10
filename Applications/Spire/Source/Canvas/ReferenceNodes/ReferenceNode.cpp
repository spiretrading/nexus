#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include <unordered_set>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

namespace {
  std::string GetReferentText(const std::string& referent) {
    if(referent.empty()) {
      return "Reference";
    }
    return "@" + referent;
  }

  std::string NormalizeReferent(std::string referent) {
    auto name = referent.find_first_not_of('<');
    if(name == std::string::npos) {
      return referent;
    }
    static const auto SECURITY = std::string("security");
    if(referent.compare(name, SECURITY.size(), SECURITY) == 0 &&
        (name + SECURITY.size() == referent.size() ||
          referent[name + SECURITY.size()] == '.')) {
      referent.replace(name, SECURITY.size(), "ticker");
    }
    return referent;
  }
}

ReferenceNode::ReferenceNode() {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(const std::string& referent)
    : m_referent(NormalizeReferent(referent)) {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(
    const std::string& referent, const CanvasType& type)
    : m_referent(NormalizeReferent(referent)) {
  SetType(type);
  SetText(GetReferentText(m_referent));
}

const std::string& ReferenceNode::GetReferent() const {
  return m_referent;
}

std::unique_ptr<CanvasNode> ReferenceNode::SetReferent(
    const std::string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = NormalizeReferent(referent);
  clone->SetText(GetReferentText(clone->m_referent));
  return clone;
}

void ReferenceNode::MigrateReferent() {
  auto normalized = NormalizeReferent(m_referent);
  if(normalized != m_referent) {
    m_referent = std::move(normalized);
    SetText(GetReferentText(m_referent));
  }
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
