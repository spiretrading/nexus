#include "Spire/Canvas/Common/FunctionNode.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  std::shared_ptr<CanvasType> MakeCompatibleType(std::size_t index,
      const std::vector<FunctionNode::Signature>& signatures) {
    auto compatibleTypes = std::vector<std::shared_ptr<NativeType>>();
    for(auto& signature : signatures) {
      compatibleTypes.push_back(signature[index]);
    }
    return UnionType::Create(MakeDereferenceView(compatibleTypes));
  }

  std::unique_ptr<CanvasNode> MakeParameter(std::size_t index,
      const std::vector<FunctionNode::Signature>& signatures) {
    auto type = MakeCompatibleType(index, signatures);
    auto parameter = MakeDefaultCanvasNode(*type);
    return parameter;
  }
}

void FunctionNode::DefineFunction(
    std::string name, std::vector<std::string> parameterNames,
    std::vector<Signature> signatures) {
  SetText(std::move(name));
  m_signatures = std::move(signatures);
  for(std::size_t i = 0; i < parameterNames.size(); ++i) {
    AddChild(std::move(parameterNames[i]), MakeParameter(i, m_signatures));
  }
  SetType(*MakeCompatibleType(m_signatures.front().size() - 1, m_signatures));
}

const std::vector<FunctionNode::Signature>&
    FunctionNode::GetSignatures() const {
  return m_signatures;
}
