#include "Spire/Canvas/Common/CanvasNode.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Operations/CanvasTypeIntersection.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

const string CanvasNode::MIME_TYPE = "application/x-spire.canvas_node";

CanvasNode::~CanvasNode() {}

boost::optional<const CanvasNode&> CanvasNode::GetParent() const {
  if(m_parent == nullptr) {
    return none;
  }
  return *m_parent;
}

View<const CanvasNode> CanvasNode::GetChildren() const {
  return MakeDereferenceView(m_children);
}

const CanvasType& CanvasNode::GetType() const {
  return *m_type;
}

const string& CanvasNode::GetText() const {
  return m_text;
}

const string& CanvasNode::GetName() const {
  return m_name;
}

bool CanvasNode::IsVisible() const {
  return m_isVisible && (m_parent == nullptr || m_parent->IsVisible());
}

bool CanvasNode::IsReadOnly() const {
  return m_isReadOnly || (m_parent != nullptr && m_parent->IsReadOnly());
}

boost::optional<const CanvasNode::MetaData&> CanvasNode::FindMetaData(
    const string& name) const {
  auto metaDataIterator = m_metaData.find(name);
  if(metaDataIterator == m_metaData.end()) {
    return none;
  }
  return metaDataIterator->second;
}

boost::optional<const CanvasNode&> CanvasNode::FindChild(
    const string& name) const {
  auto child = find_if(m_children,
    [&] (const unique_ptr<CanvasNode>& child) {
      return name == child->GetName();
    });
  if(child == m_children.end()) {
    return none;
  }
  return **child;
}

boost::optional<const CanvasNode&> CanvasNode::FindNode(
    const string& name) const {
  if(name.empty()) {
    return *this;
  }
  if(name[0] == '<') {
    if(IsRoot(*this)) {
      return none;
    }
    return GetParent()->FindNode(name.substr(1));
  }
  auto splitName = SplitName(name);
  auto child = find_if(m_children,
    [&] (const std::unique_ptr<CanvasNode>& child) {
      return child->GetName() == splitName.m_identifier;
    });
  if(child != m_children.end()) {
    return (*child)->FindNode(splitName.m_suffix);
  }
  try {
    auto index = lexical_cast<size_t>(splitName.m_identifier);
    if(index >= 0 && index < m_children.size()) {
      return m_children[index]->FindNode(splitName.m_suffix);
    }
  } catch(bad_lexical_cast&) {}
  return none;
}

unique_ptr<CanvasNode> CanvasNode::Convert(const CanvasType& type) const {
  if(IsCompatible(type, GetType())) {
    return Clone(*this);
  }
  auto intersectionType = Intersect(GetType(), type);
  if(type.GetCompatibility(*intersectionType) !=
      CanvasType::Compatibility::EQUAL) {
    if(IsCompatible(type, *intersectionType)) {
      try {
        auto conversion = Convert(*intersectionType);
        return conversion;
      } catch(CanvasOperationException&) {}
    }
  }
  auto record = dynamic_cast<const RecordType*>(&GetType());
  if(record == nullptr) {
    BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
  }
  for(const auto& field : record->GetFields()) {
    if(IsCompatible(type, *field.m_type)) {
      auto query = Spire::Convert(make_unique<QueryNode>(), type);
      auto relinkedNode = Relink(*this);
      query = query->Replace(query->GetChildren().front(),
        std::move(relinkedNode));
      return query;
    }
  }
  BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
}

unique_ptr<CanvasNode> CanvasNode::Replace(const string& child,
    unique_ptr<CanvasNode> replacement) const {
  if(auto childNode = FindChild(child)) {
    return Replace(*childNode, std::move(replacement));
  }
  BOOST_THROW_EXCEPTION(CanvasOperationException("Child not found."));
}

unique_ptr<CanvasNode> CanvasNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  auto newNode = Clone(*this);
  replacement = Spire::Convert(std::move(replacement), child.GetType());
  newNode->SetChild(child, std::move(replacement));
  return newNode;
}

unique_ptr<CanvasNode> CanvasNode::SetVisible(bool isVisible) const {
  auto clone = Clone(*this);
  clone->m_isVisible = isVisible;
  return clone;
}

unique_ptr<CanvasNode> CanvasNode::SetReadOnly(bool isReadOnly) const {
  auto clone = Clone(*this);
  clone->m_isReadOnly = isReadOnly;
  return clone;
}

unique_ptr<CanvasNode> CanvasNode::SetMetaData(const string& name,
    MetaData value) const {
  auto clone = Clone(*this);
  clone->m_metaData[name] = std::move(value);
  return clone;
}

unique_ptr<CanvasNode> CanvasNode::DeleteMetaData(const string& name) const {
  auto clone = Clone(*this);
  clone->m_metaData.erase(name);
  return clone;
}

CanvasNode::CanvasNode()
    : m_parent(nullptr),
      m_isVisible(true),
      m_isReadOnly(false) {}

CanvasNode::CanvasNode(const CanvasNode& node)
    : m_parent(nullptr),
      m_type(node.m_type),
      m_text(node.m_text),
      m_isVisible(node.m_isVisible),
      m_isReadOnly(node.m_isReadOnly),
      m_metaData(node.m_metaData) {
  for(const auto& nodeChild : node.m_children) {
    auto child = nodeChild->Clone();
    child->m_parent = this;
    child->m_name = nodeChild->GetName();
    m_children.push_back(std::move(child));
  }
}

void CanvasNode::SetType(const CanvasType& type) {
  m_type = type;
}

void CanvasNode::SetText(string text) {
  m_text = std::move(text);
}

void CanvasNode::AddChild(string name, unique_ptr<CanvasNode> node) {
  node->m_parent = this;
  node->m_name = std::move(name);
  m_children.push_back(std::move(node));
}

void CanvasNode::RemoveChild(const CanvasNode& child) {
  auto childIterator = std::find_if(m_children.begin(), m_children.end(),
    [&] (const unique_ptr<CanvasNode>& selfChild) {
      return selfChild->GetName() == child.GetName();
    });
  if(childIterator == m_children.end()) {
    BOOST_THROW_EXCEPTION(CanvasOperationException("Child not found."));
  }
  m_children.erase(childIterator);
}

void CanvasNode::SetChild(const CanvasNode& child,
    unique_ptr<CanvasNode> node) {
  auto childIterator = std::find_if(m_children.begin(), m_children.end(),
    [&] (const std::unique_ptr<CanvasNode>& selfChild) {
      return selfChild->GetName() == child.GetName();
    });
  if(childIterator == m_children.end()) {
    BOOST_THROW_EXCEPTION(CanvasOperationException("Child not found."));
  }
  node->m_parent = this;
  node->m_name = child.m_name;
  swap(*childIterator, node);
}

void CanvasNode::RenameChild(const CanvasNode& child, std::string name) {
  auto childIterator = std::find_if(m_children.begin(), m_children.end(),
    [&] (const unique_ptr<CanvasNode>& selfChild) {
      return selfChild->GetName() == child.GetName();
    });
  if(childIterator == m_children.end()) {
    BOOST_THROW_EXCEPTION(CanvasOperationException("Child not found."));
  }
  (*childIterator)->m_name = std::move(name);
}

void CanvasNode::InternalSetVisible(bool isVisible) {
  m_isVisible = isVisible;
}

void CanvasNode::InternalSetReadOnly(bool isReadOnly) {
  m_isReadOnly = isReadOnly;
}

unique_ptr<CanvasNode> CanvasNode::Reset() const {
  return MakeDefaultCanvasNode(GetType());
}
