#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasPath.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasNodeBuilder::CanvasNodeBuilder(const CanvasNode& node)
    : m_node(CanvasNode::Clone(node)) {}

unique_ptr<CanvasNode> CanvasNodeBuilder::Make() {
  return std::move(m_node);
}

void CanvasNodeBuilder::Convert(const CanvasPath& path,
    const CanvasType& type) {
  auto& node = GetNode(path, *m_node);
  if(IsCompatible(node.GetType(), type)) {
    return;
  }
  auto convertedNode = node.Convert(type);
  Replace(node, std::move(convertedNode));
}

void CanvasNodeBuilder::ForceConvert(const CanvasPath& path,
    const CanvasType& type) {
  auto& node = GetNode(path, *m_node);
  if(IsCompatible(node.GetType(), type)) {
    return;
  }
  unique_ptr<CanvasNode> convertedNode;
  try {
    convertedNode = node.Convert(type);
  } catch(std::exception&) {
    convertedNode = MakeDefaultCanvasNode(type);
  }
  Replace(node, std::move(convertedNode));
}

void CanvasNodeBuilder::Replace(const CanvasPath& path,
    unique_ptr<CanvasNode> replacement) {
  auto& node = GetNode(path, *m_node);
  if(IsRoot(node)) {
    m_node = std::move(replacement);
    return;
  }
  Replace(*node.m_parent, node.m_parent->Replace(node, std::move(replacement)));
}

void CanvasNodeBuilder::SetVisible(const CanvasPath& path, bool visible) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.SetVisible(visible));
}

void CanvasNodeBuilder::SetReadOnly(const CanvasPath& path, bool readOnly) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.SetReadOnly(readOnly));
}

void CanvasNodeBuilder::SetMetaData(const CanvasPath& path, string name,
    CanvasNode::MetaData value) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.SetMetaData(std::move(name), std::move(value)));
}

void CanvasNodeBuilder::DeleteMetaData(const CanvasPath& path,
    const string& name) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.DeleteMetaData(name));
}

void CanvasNodeBuilder::Reset(const CanvasPath& path) {
  auto& node = GetNode(path, *m_node);
  auto resetNode = node.Reset();
  Replace(node, std::move(resetNode));
}

void CanvasNodeBuilder::Commit(const CanvasNode& node,
    unique_ptr<CanvasNode> replacement) {
  if(IsRoot(node)) {
    m_node = std::move(replacement);
  } else {
    m_node = node.m_parent->Replace(node, std::move(replacement));
  }
}
