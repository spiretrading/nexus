#include "Spire/Canvas/Operations/ResolveReferences.hpp"
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  class ResolveReferencesVisitor : public CanvasNodeVisitor {
    public:
      unique_ptr<CanvasNode> Resolve(const CanvasNode& node);

    private:
      unique_ptr<CanvasNode> m_node;

      virtual void Visit(const ReferenceNode& node);
  };

  unique_ptr<CanvasNode> ResolveReferencesVisitor::Resolve(
      const CanvasNode& node) {
    m_node.reset();
    node.Apply(*this);
    return std::move(m_node);
  }

  void ResolveReferencesVisitor::Visit(const ReferenceNode& node) {
    auto currentNode = &node;
    auto referent = currentNode->FindReferent();
    if(!referent.is_initialized() ||
        IsCompatible(referent->GetType(), currentNode->GetType())) {
      return;
    }
    try {
      CanvasNodeBuilder builder(GetRoot(*currentNode));
      builder.Convert(*currentNode, referent->GetType());
      m_node = builder.Make();
    } catch(std::exception&) {}
  }
}

unique_ptr<CanvasNode> Spire::ResolveReferences(unique_ptr<CanvasNode> node) {
  auto root = std::move(node);
  ResolveReferencesVisitor visitor;
  vector<unique_ptr<CanvasNode>> revisions;
  revisions.emplace_back(CanvasNode::Clone(*root));
  BreadthFirstCanvasNodeIterator i(*root);
  while(i != BreadthFirstCanvasNodeIterator()) {
    auto update = visitor.Resolve(*i);
    if(update != nullptr) {
      auto isDuplicate = false;
      for(const auto& revision : revisions) {
        if(IsStructurallyEqual(*revision, *update)) {
          isDuplicate = true;
          break;
        }
      }
      if(isDuplicate) {
        ++i;
      } else {
        revisions.emplace_back(CanvasNode::Clone(*update));
        root = std::move(update);
        i = BreadthFirstCanvasNodeIterator(*root);
      }
    } else {
      ++i;
    }
  }
  return root;
}
