#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include <Beam/Utilities/AssertionException.hpp>
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
using namespace std;

CanvasNodeModel::Identity::Identity()
    : m_modelId(0),
      m_canvasNodeId(0) {}

CanvasNodeModel::Identity::Identity(int modelId, int canvasNodeId)
    : m_modelId(modelId),
      m_canvasNodeId(canvasNodeId) {}

string CanvasNodeModel::GetIdentityKey() {
  return "spire.identity";
}

boost::optional<CanvasNodeModel::Identity> CanvasNodeModel::FindIdentity(
    const CanvasNode& node) {
  auto identity = node.FindMetaData(GetIdentityKey());
  if(identity.is_initialized()) {
    auto value = get<const string>(&*identity);
    if(value == nullptr) {
      return none;
    }
    auto separator = value->find('-');
    if(separator == string::npos) {
      return none;
    }
    int modelId;
    int canvasNodeId;
    try {
      modelId = lexical_cast<int>(value->substr(0, separator));
      canvasNodeId = lexical_cast<int>(value->substr(separator + 1));
    } catch(const std::exception&) {
      return none;
    }
    return Identity(modelId, canvasNodeId);
  }
  return none;
}

string CanvasNodeModel::ToMetaData(const Identity& identity) {
  return lexical_cast<string>(identity.m_modelId) + "-" +
    lexical_cast<string>(identity.m_canvasNodeId);
}

string CanvasNodeModel::GetReferentKey() {
  return "spire.referent";
}

boost::optional<const CanvasNode&> CanvasNodeModel::FindReferent(
    const ReferenceNode& node) {
  auto referentKey = node.FindMetaData(GetReferentKey());
  if(!referentKey.is_initialized()) {
    return none;
  }
  auto referentValue = get<const string>(&*referentKey);
  if(referentValue == nullptr) {
    return none;
  }
  for(const auto& referent : BreadthFirstView(GetRoot(node))) {
    auto identity = FindIdentity(referent);
    if(identity.is_initialized() && ToMetaData(*identity) == *referentValue) {
      return referent;
    }
  }
  return none;
}

unique_ptr<CanvasNode> CanvasNodeModel::StripIdentity(const CanvasNode& node) {
  CanvasNodeBuilder builder(node);
  for(const auto& stripNode : BreadthFirstView(node)) {
    builder.DeleteMetaData(stripNode, CanvasNodeModel::GetIdentityKey());
  }
  return builder.Make();
}

CanvasNodeModel::Coordinate::Coordinate(int row, int column)
    : m_row(row),
      m_column(column) {}

bool CanvasNodeModel::Coordinate::operator <(
    const Coordinate& coordinate) const {
  return std::tie(m_row, m_column) <
    std::tie(coordinate.m_row, coordinate.m_column);
}

bool CanvasNodeModel::Coordinate::operator ==(
    const Coordinate& coordinate) const {
  return m_row == coordinate.m_row && m_column == coordinate.m_column;
}

bool CanvasNodeModel::Coordinate::operator !=(
    const Coordinate& coordinate) const {
  return !(*this == coordinate);
}

void CanvasNodeModel::Snapshot::Save(const CanvasNodeModel& model) {
  m_roots.clear();
  for(const auto& node : model.GetRoots()) {
    auto coordinate = model.GetCoordinate(*node);
    auto root = std::make_tuple(coordinate, CanvasNode::Clone(*node));
    m_roots.emplace_back(std::move(root));
  }
}

void CanvasNodeModel::Snapshot::Restore(Out<CanvasNodeModel> model) {
  while(!model->GetRoots().empty()) {
    auto root = model->GetRoots().back();
    model->Remove(*root);
  }
  for(const auto& root : m_roots) {
    model->Add(get<0>(root), *get<1>(root));
  }
}

boost::optional<const CanvasNode&> CanvasNodeModel::GetCurrentNode() const {
  auto coordinate = GetCurrentCoordinate();
  if(!coordinate) {
    return none;
  }
  return GetNode(*coordinate);
}

boost::optional<CanvasNodeModel::Coordinate> CanvasNodeModel::
    GetCurrentCoordinate() const {
  auto node = GetCurrentNode();
  if(!node.is_initialized()) {
    return none;
  }
  return GetCoordinate(*node);
}

void CanvasNodeModel::SetCurrent(const CanvasNode& node) {
  auto coordinate = GetCoordinate(node);
  SetCurrent(coordinate);
}

void CanvasNodeModel::SetCurrent(const Coordinate& coordinate) {
  auto node = GetNode(coordinate);
  BEAM_ASSERT(node);
  SetCurrent(*node);
}

void CanvasNodeModel::Remove(const CanvasNode& node) {
  auto coordinate = GetCoordinate(node);
  Remove(coordinate);
}

void CanvasNodeModel::Remove(const Coordinate& coordinate) {
  auto node = GetNode(coordinate);
  BEAM_ASSERT(node);
  Remove(*node);
}
