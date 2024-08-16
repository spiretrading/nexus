#include "Spire/CanvasView/LocalCanvasNodeModel.hpp"
#include <algorithm>
#include "Spire/Canvas/Common/CanvasNode.hpp"

using namespace boost;
using namespace Spire;

std::vector<const CanvasNode*> LocalCanvasNodeModel::GetRoots() const {
  auto roots = std::vector<const CanvasNode*>();
  std::transform(m_roots.begin(), m_roots.end(), std::back_inserter(roots),
    [] (const auto& root) {
      return root.get();
    });
  return roots;
}

optional<const CanvasNode&>
    LocalCanvasNodeModel::GetNode(const Coordinate& coordinate) const {
  auto i = m_coordinateToNode.find(coordinate);
  if(i == m_coordinateToNode.end()) {
    return none;
  }
  return *i->second;
}

LocalCanvasNodeModel::Coordinate
    LocalCanvasNodeModel::GetCoordinate(const CanvasNode& node) const {
  auto i = m_nodeToCoordinate.find(&node);
  if(i == m_nodeToCoordinate.end()) {
    return Coordinate(-1, -1);
  }
  return i->second;
}

optional<LocalCanvasNodeModel::Coordinate>
    LocalCanvasNodeModel::GetCurrentCoordinate() const {
  return m_current;
}

void LocalCanvasNodeModel::SetCurrent(const Coordinate& coordinate) {
  m_current = coordinate;
}

const CanvasNode& LocalCanvasNodeModel::Add(
    const Coordinate& coordinate, const CanvasNode& node) {
  auto internalNode = CanvasNode::Clone(node);
  InternalAdd(coordinate, *internalNode);
  m_roots.push_back(std::move(internalNode));
  return *m_roots.back();
}

void LocalCanvasNodeModel::Remove(const CanvasNode& node) {
  auto coordinate = GetCoordinate(node);
  if(coordinate.m_row == -1 || coordinate.m_column == -1 || node.GetParent()) {
    return;
  }
  InternalRemove(node);
  std::erase_if(m_roots, [&] (const auto& root) {
    return root.get() == &node;
  });
}

void LocalCanvasNodeModel::InternalAdd(
    const Coordinate& coordinate, const CanvasNode& node) {
  m_coordinateToNode.insert(std::pair(coordinate, &node));
  m_nodeToCoordinate.insert(std::pair(&node, coordinate));
  auto currentCoordinate =
    Coordinate(coordinate.m_row, coordinate.m_column + 1);
  for(auto& child : node.GetChildren()) {
    InternalAdd(currentCoordinate, child);
    ++currentCoordinate.m_row;
  }
}

void LocalCanvasNodeModel::InternalRemove(const CanvasNode& node) {
  for(auto& child : node.GetChildren()) {
    InternalRemove(child);
  }
  auto i = m_nodeToCoordinate.find(&node);
  m_coordinateToNode.erase(i->second);
  m_nodeToCoordinate.erase(i);
}
