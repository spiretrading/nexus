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
      const CanvasType& returnType, std::size_t index) {
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

  auto MergeSignatureTypes(
      const std::vector<SignatureNode::Signature>& signatures,
      std::size_t index) {
    auto types = std::vector<std::shared_ptr<NativeType>>();
    for(auto& signature : signatures) {
      types.push_back(signature[index]);
    }
    return UnionType::Create(MakeDereferenceView(types));
  }
}

std::unique_ptr<CanvasNode>
    SignatureNode::Convert(const CanvasType& type) const {
  if(type.GetCompatibility(GetType()) == CanvasType::Compatibility::EQUAL) {
    return Clone(*this);
  }
  auto compatibleSignatures = std::vector<Signature>();
  for(auto& signature : GetSignatures()) {
    if(IsCompatible(type, *signature.back())) {
      auto clone = Clone(*this);
      auto isCompatible = true;
      for(auto i = std::size_t(0); i != signature.size() - 1; ++i) {
        auto& parameterType = signature[i];
        auto& child = clone->GetChildren()[i];
        if(!IsCompatible(*parameterType, clone->GetChildren()[i].GetType())) {
          try {
            clone->SetChild(
              child, Spire::Convert(Clone(child), *parameterType));
          } catch(const CanvasOperationException&) {
            isCompatible = false;
            break;
          }
        }
      }
      if(isCompatible) {
        compatibleSignatures.push_back(signature);
      }
    }
  }
  if(compatibleSignatures.empty()) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  auto clone = Clone(*this);
  for(auto i = 0; i != compatibleSignatures.front().size() - 1; ++i) {
    auto parameterType = MergeSignatureTypes(compatibleSignatures, i);
    auto& child = clone->GetChildren()[i];
    if(!IsCompatible(*parameterType, child.GetType())) {
      clone->SetChild(child, ForceConversion(Clone(child), *parameterType));
    }
  }
  auto returnType = MergeSignatureTypes(
    compatibleSignatures, compatibleSignatures.front().size() - 1);
  if(!IsCompatible(*returnType, clone->GetType())) {
    clone->SetType(*returnType);
  }
  return clone;
}

std::unique_ptr<CanvasNode> SignatureNode::Replace(
    const CanvasNode& child, std::unique_ptr<CanvasNode> replacement) const {
  auto clone = CanvasNode::Clone(*this);
  if(child.GetType().GetCompatibility(replacement->GetType()) ==
      CanvasType::Compatibility::EQUAL) {
    clone->SetChild(child, std::move(replacement));
    return clone;
  }
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
  auto& replacementType = replacement->GetType();
  clone->SetChild(child, std::move(replacement));
  auto remainingSignatures = std::vector<Signature>();
  for(auto& signature : GetSignatures()) {
    auto isCompatible = true;
    for(auto i = std::size_t(0); i != signature.size() - 1; ++i) {
      if(!IsCompatible(clone->GetChildren()[i].GetType(), *signature[i])) {
        isCompatible = false;
        break;
      }
    }
    if(isCompatible) {
      remainingSignatures.push_back(signature);
    }
  }
  for(auto i = std::size_t(0); i < GetChildren().size(); ++i) {
    auto& arg = clone->GetChildren()[i];
    auto argParameterType =
      GetSignatureType(remainingSignatures, *clone->m_type, i);
    if(!IsCompatible(*argParameterType, arg.GetType())) {
      auto convertedArg = ForceConversion(Clone(arg), *argParameterType);
      clone->SetChild(arg, std::move(convertedArg));
    }
  }
  auto returnType = GetReturnType(*clone, GetSignatures());
  clone->SetType(*returnType);
  return clone;
}

SignatureNode::SignatureNode()
  : m_type(UnionType::GetAnyType()) {}
