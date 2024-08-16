#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include <Beam/Utilities/AssertionException.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

CanvasNodeModel::Identity::Identity()
  : m_modelId(0),
    m_canvasNodeId(0) {}

CanvasNodeModel::Identity::Identity(int modelId, int canvasNodeId)
  : m_modelId(modelId),
    m_canvasNodeId(canvasNodeId) {}

std::string CanvasNodeModel::GetIdentityKey() {
  return "spire.identity";
}

optional<CanvasNodeModel::Identity>
    CanvasNodeModel::FindIdentity(const CanvasNode& node) {
  if(auto identity = node.FindMetaData(GetIdentityKey())) {
    auto value = get<const std::string>(&*identity);
    if(!value) {
      return none;
    }
    auto separator = value->find('-');
    if(separator == std::string::npos) {
      return none;
    }
    try {
      auto modelId = lexical_cast<int>(value->substr(0, separator));
      auto canvasNodeId = lexical_cast<int>(value->substr(separator + 1));
      return Identity(modelId, canvasNodeId);
    } catch(const std::exception&) {
      return none;
    }
  }
  return none;
}

std::string CanvasNodeModel::ToMetaData(const Identity& identity) {
  return lexical_cast<std::string>(identity.m_modelId) + "-" +
    lexical_cast<std::string>(identity.m_canvasNodeId);
}

std::string CanvasNodeModel::GetReferentKey() {
  return "spire.referent";
}

optional<const CanvasNode&>
    CanvasNodeModel::FindReferent(const ReferenceNode& node) {
  auto referentKey = node.FindMetaData(GetReferentKey());
  if(!referentKey) {
    return none;
  }
  auto referentValue = get<const std::string>(&*referentKey);
  if(!referentValue) {
    return none;
  }
  for(auto& referent : BreadthFirstView(GetRoot(node))) {
    auto identity = FindIdentity(referent);
    if(identity && ToMetaData(*identity) == *referentValue) {
      return referent;
    }
  }
  return none;
}

std::unique_ptr<CanvasNode>
    CanvasNodeModel::StripIdentity(const CanvasNode& node) {
  auto builder = CanvasNodeBuilder(node);
  for(auto& stripNode : BreadthFirstView(node)) {
    builder.DeleteMetaData(stripNode, CanvasNodeModel::GetIdentityKey());
  }
  return builder.Make();
}

void CanvasNodeModel::Snapshot::Save(const CanvasNodeModel& model) {
  m_roots.clear();
  for(auto& node : model.GetRoots()) {
    auto coordinate = model.GetCoordinate(*node);
    auto root = std::tuple(coordinate, CanvasNode::Clone(*node));
    m_roots.emplace_back(std::move(root));
  }
}

void CanvasNodeModel::Snapshot::Restore(Out<CanvasNodeModel> model) {
  while(!model->GetRoots().empty()) {
    auto root = model->GetRoots().back();
    model->Remove(*root);
  }
  for(auto& root : m_roots) {
    model->Add(std::get<0>(root), *std::get<1>(root));
  }
}

optional<const CanvasNode&> CanvasNodeModel::GetCurrentNode() const {
  if(auto coordinate = GetCurrentCoordinate()) {
    return GetNode(*coordinate);
  }
  return none;
}

optional<CanvasNodeModel::Coordinate>
    CanvasNodeModel::GetCurrentCoordinate() const {
  if(auto node = GetCurrentNode()) {
    return GetCoordinate(*node);
  }
  return none;
}

void CanvasNodeModel::SetCurrent(const CanvasNode& node) {
  SetCurrent(GetCoordinate(node));
}

void CanvasNodeModel::SetCurrent(const Coordinate& coordinate) {
  auto node = GetNode(coordinate);
  BEAM_ASSERT(node);
  SetCurrent(*node);
}

void CanvasNodeModel::Remove(const CanvasNode& node) {
  Remove(GetCoordinate(node));
}

void CanvasNodeModel::Remove(const Coordinate& coordinate) {
  auto node = GetNode(coordinate);
  BEAM_ASSERT(node);
  Remove(*node);
}

std::ostream& Spire::operator <<(
    std::ostream& out, const CanvasNodeModel::Coordinate& coordinate) {
  return out << '(' << coordinate.m_row << ", " << coordinate.m_column << ')';
}

size_t std::hash<CanvasNodeModel::Coordinate>::operator()(
    const CanvasNodeModel::Coordinate& value) const {
  auto seed = std::size_t(0);
  hash_combine(seed, value.m_row);
  hash_combine(seed, value.m_column);
  return seed;
}
