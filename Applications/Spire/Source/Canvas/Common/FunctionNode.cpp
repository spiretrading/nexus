#include "Spire/Canvas/Common/FunctionNode.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<CanvasType> BuildCompatibleType(size_t index,
      const vector<FunctionNode::Signature>& signatures) {
    vector<std::shared_ptr<NativeType>> compatibleTypes;
    for(const auto& signature : signatures) {
      compatibleTypes.push_back(signature[index]);
    }
    return UnionType::Create(MakeDereferenceView(compatibleTypes));
  }

  unique_ptr<CanvasNode> BuildParameter(size_t index,
      const vector<FunctionNode::Signature>& signatures) {
    auto type = BuildCompatibleType(index, signatures);
    auto parameter = BuildDefaultCanvasNode(*type);
    return parameter;
  }
}

void FunctionNode::DefineFunction(string name, vector<string> parameterNames,
    vector<Signature> signatures) {
  SetText(std::move(name));
  m_signatures = std::move(signatures);
  for(size_t i = 0; i < parameterNames.size(); ++i) {
    AddChild(std::move(parameterNames[i]), BuildParameter(i, m_signatures));
  }
  SetType(*BuildCompatibleType(m_signatures.front().size() - 1, m_signatures));
}

const vector<FunctionNode::Signature>& FunctionNode::GetSignatures() const {
  return m_signatures;
}
