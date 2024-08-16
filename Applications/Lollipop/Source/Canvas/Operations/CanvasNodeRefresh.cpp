#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/ResolveReferences.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

unique_ptr<CanvasNode> Spire::Refresh(unique_ptr<CanvasNode> node) {
  node = Convert(ResolveReferences(std::move(node)), UnionType::GetAnyType());
  CanvasNodeBuilder builder(*node);
  for(const auto& child : BreadthFirstView(*node)) {
    if(auto proxy = dynamic_cast<const ProxyNode*>(&child)) {
      builder.Replace(child, CanvasNode::Clone(proxy->GetOriginal()));
    }
  }
  return builder.Make();
}
