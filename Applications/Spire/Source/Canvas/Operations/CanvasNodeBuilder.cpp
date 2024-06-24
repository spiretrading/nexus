#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasPath.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace boost;
using namespace Spire;

CanvasNodeBuilder::CanvasNodeBuilder(const CanvasNode& node)
  : m_node(CanvasNode::Clone(node)) {}

std::unique_ptr<CanvasNode> CanvasNodeBuilder::Make() const {
  return CanvasNode::Clone(*m_node);
}

void CanvasNodeBuilder::Convert(
    const CanvasPath& path, const CanvasType& type) {
  auto& node = GetNode(path, *m_node);
  if(IsCompatible(node.GetType(), type)) {
    return;
  }
  Replace(node, node.Convert(type));
}

void CanvasNodeBuilder::ForceConvert(
    const CanvasPath& path, const CanvasType& type) {
  auto& node = GetNode(path, *m_node);
  if(IsCompatible(node.GetType(), type)) {
    return;
  }
  auto converted_node = [&] {
    try {
      return node.Convert(type);
    } catch(const std::exception&) {
      return MakeDefaultCanvasNode(type);
    }
  }();
  Replace(node, std::move(converted_node));
}

void CanvasNodeBuilder::Replace(
    const CanvasPath& path, std::unique_ptr<CanvasNode> replacement) {
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

void CanvasNodeBuilder::SetMetaData(
    const CanvasPath& path, std::string name, CanvasNode::MetaData value) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.SetMetaData(std::move(name), std::move(value)));
}

void CanvasNodeBuilder::DeleteMetaData(
    const CanvasPath& path, const std::string& name) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.DeleteMetaData(name));
}

void CanvasNodeBuilder::Reset(const CanvasPath& path) {
  auto& node = GetNode(path, *m_node);
  Replace(node, node.Reset());
}

void CanvasNodeBuilder::Commit(
    const CanvasNode& node, std::unique_ptr<CanvasNode> replacement) {
  if(IsRoot(node)) {
    m_node = std::move(replacement);
  } else {
    m_node = node.m_parent->Replace(node, std::move(replacement));
  }
}
