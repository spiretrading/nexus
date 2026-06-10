#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include <unordered_set>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
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

  string NormalizeReferent(string referent) {
    auto name = referent.find_first_not_of('<');
    if(name == string::npos) {
      return referent;
    }
    static const auto SECURITY = string("security");
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

ReferenceNode::ReferenceNode(const string& referent)
    : m_referent(NormalizeReferent(referent)) {
  SetType(UnionType::GetAnyType());
  SetText(GetReferentText(m_referent));
}

ReferenceNode::ReferenceNode(const string& referent, const CanvasType& type)
    : m_referent(NormalizeReferent(referent)) {
  SetType(type);
  SetText(GetReferentText(m_referent));
}

const string& ReferenceNode::GetReferent() const {
  return m_referent;
}

unique_ptr<CanvasNode> ReferenceNode::SetReferent(
    const string& referent) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_referent = NormalizeReferent(referent);
  clone->SetText(GetReferentText(clone->m_referent));
  return std::move(clone);
}

void ReferenceNode::MigrateReferent() {
  auto normalized = NormalizeReferent(m_referent);
  if(normalized != m_referent) {
    m_referent = std::move(normalized);
    SetText(GetReferentText(m_referent));
  }
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

std::unique_ptr<CanvasNode> ReferenceNode::Clone() const {
  return std::make_unique<ReferenceNode>(*this);
}

std::unique_ptr<CanvasNode> ReferenceNode::Reset() const {
  return std::make_unique<ReferenceNode>();
}

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
