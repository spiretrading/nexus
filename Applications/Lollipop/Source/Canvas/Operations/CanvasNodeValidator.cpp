#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include <unordered_set>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  boost::optional<const ReferenceNode&> FindCycle(
      const ReferenceNode& reference,
      unordered_set<const ReferenceNode*> visitedReferences =
      unordered_set<const ReferenceNode*>()) {
    if(!visitedReferences.insert(&reference).second) {
      return reference;
    }
    auto anchor = FindAnchor(reference);
    if(!anchor.is_initialized()) {
      return none;
    }
    for(const auto& node : BreadthFirstView(*anchor)) {
      auto referenceCheck = dynamic_cast<const ReferenceNode*>(&node);
      if(referenceCheck != nullptr &&
          referenceCheck->GetType().GetCompatibility(
          OrderReferenceType::GetInstance()) !=
          CanvasType::Compatibility::EQUAL) {
        auto cycle = FindCycle(*referenceCheck, visitedReferences);
        if(cycle.is_initialized()) {
          if(visitedReferences.size() == 1) {
            if(&*cycle == &reference) {
              return reference;
            } else {
              return none;
            }
          } else {
            return cycle;
          }
        }
      }
    }
    return none;
  }

  bool ContainsCycle(const ReferenceNode& node) {
    auto anchor = FindAnchor(node);
    if(!anchor.is_initialized()) {
      return false;
    }
    if(anchor->GetType().GetCompatibility(OrderReferenceType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      return false;
    }
    auto monitorNode = dynamic_cast<const BlotterTaskMonitorNode*>(&*anchor);
    if(monitorNode != nullptr && IsRoot(*monitorNode)) {
      return false;
    }
    if(FindCycle(node)) {
      return true;
    }
    return IsParent(*anchor, node);
  }

  class CanvasNodeValidator : private CanvasNodeVisitor {
    public:
      std::vector<CanvasNodeValidationError> Validate(const CanvasNode& node);

    private:
      std::vector<CanvasNodeValidationError> m_errors;

      bool InternalValidate(const CanvasNode& node);
      virtual void Visit(const CanvasNode& node);
      virtual void Visit(const BlotterTaskMonitorNode& node);
      virtual void Visit(const FilePathNode& node);
      virtual void Visit(const FoldNode& node);
      virtual void Visit(const QueryNode& node);
      virtual void Visit(const ReferenceNode& node);
  };
}

vector<CanvasNodeValidationError> Spire::Validate(const CanvasNode& node) {
  CanvasNodeValidator validator;
  return validator.Validate(node);
}

vector<CanvasNodeValidationError> CanvasNodeValidator::Validate(
    const CanvasNode& node) {
  m_errors.clear();
  InternalValidate(node);
  return m_errors;
}

bool CanvasNodeValidator::InternalValidate(const CanvasNode& node) {
  auto currentSize = m_errors.size();
  node.Apply(*this);
  return m_errors.size() == currentSize;
}

void CanvasNodeValidator::Visit(const BlotterTaskMonitorNode& node) {
  for(const auto& child : node.GetChildren()) {
    InternalValidate(child);
  }
}

void CanvasNodeValidator::Visit(const FilePathNode& node) {
  if(node.GetPath().empty()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "No file path provided."));
    return;
  }
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeValidator::Visit(const FoldNode& node) {
  auto& combiner = node.GetChildren().front();
  if(combiner.GetChildren().size() < 2) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner must have two parameters."));
    return;
  }
  if(combiner.GetChildren().size() > 2) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner may only have only two parameters."));
    return;
  }
  auto leftOperand = node.FindLeftOperand();
  if(!leftOperand.is_initialized()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner requires two parameters."));
    return;
  }
  if(!leftOperand.is_initialized()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner requires two parameters."));
    return;
  }
  if(leftOperand->GetType().GetCompatibility(combiner.GetType()) !=
      CanvasType::Compatibility::EQUAL) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner's first parameter type is incompatible."));
    return;
  }
  auto& source = node.GetChildren().back();
  auto rightOperand = node.FindRightOperand();
  if(!rightOperand.is_initialized()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner requires two parameters."));
    return;
  }
  if(rightOperand->GetType().GetCompatibility(source.GetType()) !=
      CanvasType::Compatibility::EQUAL) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "combiner's second parameter type is incompatible."));
    return;
  }
}

void CanvasNodeValidator::Visit(const QueryNode& node) {
  if(node.GetField().empty()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "No field selected to query."));
    return;
  }
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeValidator::Visit(const ReferenceNode& node) {
  if(node.GetReferent().empty()) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Unspecified reference."));
    return;
  }
  auto referent = node.FindReferent();
  if(!referent) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Referent not found: " + node.GetReferent()));
    return;
  }
  if(IsSame(referent, node)) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Self reference is invalid"));
    return;
  } else if(ContainsCycle(node)) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Reference contains a cycle."));
    return;
  }
  if(dynamic_cast<const BlotterTaskMonitorNode*>(&*referent) == nullptr &&
      !IsCompatible(node.GetType(), referent->GetType())) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Referent's type does not match."));
    return;
  }
}

void CanvasNodeValidator::Visit(const CanvasNode& node) {
  if(dynamic_cast<const NativeType*>(&node.GetType()) == nullptr) {
    m_errors.push_back(CanvasNodeValidationError(Ref(node),
      "Node is not complete."));
  }
  for(const auto& node : node.GetChildren()) {
    InternalValidate(node);
  }
}
