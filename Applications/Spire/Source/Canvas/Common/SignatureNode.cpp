#include "Spire/Canvas/Common/SignatureNode.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include <Beam/Collections/IndexedIterator.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<CanvasType> GetSignatureType(
      const vector<SignatureNode::Signature>& signatures,
      const CanvasType& returnType, size_t index) {
    vector<std::shared_ptr<NativeType>> compatibleTypes;
    for(const auto& signature : signatures) {
      if(IsCompatible(returnType, *signature.back())) {
        compatibleTypes.push_back(signature[index]);
      }
    }
    return UnionType::Create(MakeDereferenceView(compatibleTypes));
  }

  std::shared_ptr<CanvasType> GetReturnType(const CanvasNode& node,
      const vector<SignatureNode::Signature>& signatures) {
    vector<std::shared_ptr<NativeType>> returnTypes;
    for(const auto& signature : signatures) {
      bool validSignature = true;
      for(const auto& child : MakeIndexedView(node.GetChildren())) {
        if(!IsCompatible(child.GetValue().GetType(),
            *signature[child.GetIndex()]) &&
            (dynamic_cast<const RecordType*>(&child.GetValue().GetType()) ==
            nullptr || std::dynamic_pointer_cast<const RecordType>(
            signature[child.GetIndex()]) == nullptr)) {
          validSignature = false;
          break;
        }
      }
      if(validSignature) {
        returnTypes.emplace_back(signature.back());
      }
    }
    return UnionType::Create(MakeDereferenceView(returnTypes));
  }
}

unique_ptr<CanvasNode> SignatureNode::Convert(const CanvasType& type) const {
  vector<SignatureNode::Signature> signatureEntries(
    GetSignatures().front().size());
  for(const auto& signature : GetSignatures()) {
    if(IsCompatible(type, *signature.back())) {
      for(const auto& type : MakeIndexedView(signature)) {
        signatureEntries[type.GetIndex()].push_back(type.GetValue());
      }
    }
  }
  if(signatureEntries.front().empty()) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  auto clone = Clone(*this);
  for(const auto& signature : DropLast(MakeIndexedView(signatureEntries))) {
    auto parameterType = UnionType::Create(
      MakeDereferenceView(signature.GetValue()));
    auto& child = clone->GetChildren()[signature.GetIndex()];
    clone->SetChild(child, ForceConversion(Clone(child), *parameterType));
  }
  auto returnType = UnionType::Create(MakeDereferenceView(
    signatureEntries.back()));
  if(!IsCompatible(*returnType, clone->GetType())) {
    clone->SetType(*returnType);
  }
  clone->m_type = returnType;
  return std::move(clone);
}

unique_ptr<CanvasNode> SignatureNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  size_t replacementIndex;
  for(const auto& selfChild : MakeIndexedView(GetChildren())) {
    if(&selfChild.GetValue() == &child) {
      replacementIndex = selfChild.GetIndex();
      break;
    }
  }
  auto replacementParameterType = GetSignatureType(GetSignatures(), *m_type,
    replacementIndex);
  if(!IsCompatible(*replacementParameterType, replacement->GetType())) {
    auto convertedReplacement = Spire::Convert(std::move(replacement),
      *replacementParameterType);
    return Replace(child, std::move(convertedReplacement));
  }
  auto clone = CanvasNode::Clone(*this);
  auto& replacementType = replacement->GetType();
  clone->SetChild(child, std::move(replacement));
  vector<Signature> remainingSignatures;
  for(const auto& signature : GetSignatures()) {
    if(IsCompatible(replacementType, *signature[replacementIndex]) ||
        dynamic_cast<const RecordType*>(&replacementType) &&
        std::dynamic_pointer_cast<const RecordType>(
        signature[replacementIndex])) {
      remainingSignatures.push_back(signature);
    }
  }
  for(size_t i = 0; i < GetChildren().size(); ++i) {
    if(i == replacementIndex) {
      continue;
    }
    auto& arg = GetChildren()[i];
    auto argParameterType = GetSignatureType(remainingSignatures, *m_type, i);
    if(!IsCompatible(*argParameterType, arg.GetType())) {
      auto convertedArg = ForceConversion(Clone(arg), *argParameterType);
      clone->SetChild(arg, std::move(convertedArg));
    }
  }
  auto returnType = GetReturnType(*clone, GetSignatures());
  clone->SetType(*returnType);
  return std::move(clone);
}

SignatureNode::SignatureNode()
    : m_type(UnionType::GetAnyType()) {}
