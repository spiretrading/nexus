#include "Spire/Canvas/ControlNodes/SpawnNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  vector<SpawnNode::Signature> BuildSpawnSignatures() {
    vector<SpawnNode::Signature> signatures;
    const auto& valueTypes = UnionType::GetAnyValueType().GetCompatibleTypes();
    for(const auto& type : valueTypes) {
      SpawnNode::Signature signature;
      signature.emplace_back(type);
      signature.emplace_back(OrderReferenceType::GetInstance());
      signature.emplace_back(OrderReferenceType::GetInstance());
      signatures.emplace_back(std::move(signature));
    }
    return signatures;
  }
}

SpawnNode::SpawnNode() {
  SetText("Spawn");
  SetType(OrderReferenceType::GetInstance());
  AddChild("trigger", make_unique<NoneNode>(UnionType::GetAnyValueType()));
  AddChild("task", make_unique<NoneNode>(OrderReferenceType::GetInstance()));
}

void SpawnNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> SpawnNode::Clone() const {
  return make_unique<SpawnNode>(*this);
}

const vector<SpawnNode::Signature>& SpawnNode::GetSignatures() const {
  static auto signatures = BuildSpawnSignatures();
  return signatures;
}

SpawnNode::SpawnNode(ReceiveBuilder) {}
