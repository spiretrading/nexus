#include "Spire/Canvas/Common/SignatureNode.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include <Beam/Collections/IndexIterator.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  auto GetSignatureType(const std::vector<SignatureNode::Signature>& signatures,
      const CanvasType& returnType, size_t index) {
    auto compatibleTypes = std::vector<std::shared_ptr<NativeType>>();
    for(auto& signature : signatures) {
      if(IsCompatible(returnType, *signature.back())) {
        compatibleTypes.push_back(signature[index]);
      }
    }
    return UnionType::Create(MakeDereferenceView(compatibleTypes));
  }

  auto GetReturnType(const CanvasNode& node,
      const std::vector<SignatureNode::Signature>& signatures) {
    auto returnTypes = std::vector<std::shared_ptr<NativeType>>();
    for(auto& signature : signatures) {
      auto validSignature = true;
      for(auto& child : MakeIndexView(node.GetChildren())) {
        if(!IsCompatible(
            child.GetValue().GetType(), *signature[child.GetIndex()]) &&
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

std::unique_ptr<CanvasNode>
    SignatureNode::Convert(const CanvasType& type) const {
  auto signatureEntries =
    std::vector<SignatureNode::Signature>(GetSignatures().front().size());
  for(auto& signature : GetSignatures()) {
    if(IsCompatible(type, *signature.back())) {
      for(const auto& type : MakeIndexView(signature)) {
        signatureEntries[type.GetIndex()].push_back(type.GetValue());
      }
    }
  }
  if(signatureEntries.front().empty()) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  auto clone = Clone(*this);
  for(auto& signature : DropLast(MakeIndexView(signatureEntries))) {
    auto parameterType =
      UnionType::Create(MakeDereferenceView(signature.GetValue()));
    auto& child = clone->GetChildren()[signature.GetIndex()];
    clone->SetChild(child, ForceConversion(Clone(child), *parameterType));
  }
  auto returnType =
    UnionType::Create(MakeDereferenceView(signatureEntries.back()));
  auto hasCompatibility = false;
  for(auto& signature : GetSignatures()) {
    if(returnType->GetCompatibility(*signature.back()) ==
        CanvasType::Compatibility::EQUAL) {
      auto isCompatible = true;
      for(const auto& parameter : DropLast(MakeIndexView(signature))) {
        auto& child = clone->GetChildren()[parameter.GetIndex()];
        if(!IsCompatible(child.GetType(), *parameter.GetValue())) {
          isCompatible = false;
          break;
        }
      }
      if(isCompatible) {
        hasCompatibility = true;
        break;
      }
    }
  }
  if(!hasCompatibility) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  if(!IsCompatible(*returnType, clone->GetType())) {
    clone->SetType(*returnType);
  }
  clone->m_type = returnType;
  return std::move(clone);
}

std::unique_ptr<CanvasNode> SignatureNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto replacementIndex = [&] {
    for(auto& selfChild : MakeIndexView(GetChildren())) {
      if(&selfChild.GetValue() == &child) {
        return selfChild.GetIndex();
      }
    }
    BOOST_THROW_EXCEPTION(CanvasOperationException("Child not found."));
  }();
  auto replacementParameterType =
    GetSignatureType(GetSignatures(), *m_type, replacementIndex);
  if(!IsCompatible(*replacementParameterType, replacement->GetType())) {
    auto convertedReplacement =
      Spire::Convert(std::move(replacement), *replacementParameterType);
    return Replace(child, std::move(convertedReplacement));
  }
  auto clone = CanvasNode::Clone(*this);
  auto& replacementType = replacement->GetType();
  clone->SetChild(child, std::move(replacement));
  auto remainingSignatures = std::vector<Signature>();
  for(auto& signature : GetSignatures()) {
    if(IsCompatible(replacementType, *signature[replacementIndex]) ||
        dynamic_cast<const RecordType*>(&replacementType) &&
          std::dynamic_pointer_cast<const RecordType>(
            signature[replacementIndex])) {
      remainingSignatures.push_back(signature);
    }
  }
  for(auto i = std::size_t(0); i < GetChildren().size(); ++i) {
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
