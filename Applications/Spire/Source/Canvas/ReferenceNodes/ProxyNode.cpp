#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

ProxyNode::ProxyNode(const std::string& referent, const CanvasType& type,
    std::unique_ptr<CanvasNode> original)
    : ReferenceNode(referent, type),
      m_original(std::move(original)) {
  InternalSetReadOnly(true);
  InternalSetVisible(false);
}

const CanvasNode& ProxyNode::GetOriginal() const {
  return *m_original;
}

std::unique_ptr<CanvasNode> ProxyNode::Convert(const CanvasType& type) const {
  return std::make_unique<ProxyNode>(
    GetReferent(), type, CanvasNode::Clone(*m_original));
}

std::unique_ptr<CanvasNode> ProxyNode::SetVisible(bool value) const {
  return CanvasNode::Clone(*this);
}

std::unique_ptr<CanvasNode> ProxyNode::SetReadOnly(bool value) const {
  return CanvasNode::Clone(*this);
}

void ProxyNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> ProxyNode::Clone() const {
  return std::make_unique<ProxyNode>(*this);
}

std::unique_ptr<CanvasNode> ProxyNode::Reset() const {
  return std::make_unique<ProxyNode>(*this);
}
