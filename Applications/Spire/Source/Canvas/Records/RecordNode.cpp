#include "Spire/Canvas/Records/RecordNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

RecordNode::RecordNode(const RecordType& type) {
  for(const auto& field : type.GetFields()) {
    AddChild(field.m_name, MakeDefaultCanvasNode(*field.m_type));
  }
  SetText(type.GetName());
  SetType(type);
}

void RecordNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> RecordNode::Clone() const {
  return make_unique<RecordNode>(*this);
}

unique_ptr<CanvasNode> RecordNode::Reset() const {
  return make_unique<RecordNode>(static_cast<const RecordType&>(GetType()));
}
