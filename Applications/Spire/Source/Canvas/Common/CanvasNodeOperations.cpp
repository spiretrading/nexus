#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include <Beam/Serialization/TypeRegistry.hpp>
#include <boost/regex.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

unique_ptr<QMimeData> Spire::EncodeAsMimeData(
    const View<const CanvasNode>& nodes) {
  vector<const CanvasNode*> clonedNodes;
  transform(nodes.begin(), nodes.end(), back_inserter(clonedNodes),
    [&] (const CanvasNode& node) {
      return &node;
    });
  auto mimeData = make_unique<QMimeData>();
  TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
  RegisterSpireTypes(Store(typeRegistry));
  auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
  SharedBuffer buffer;
  sender.SetSink(Ref(buffer));
  sender.Shuttle(clonedNodes);
  QByteArray encodedData(buffer.GetData(), buffer.GetSize());
  mimeData->setData(QString::fromStdString(CanvasNode::MIME_TYPE), encodedData);
  return mimeData;
}

vector<unique_ptr<CanvasNode>> Spire::DecodeFromMimeData(
    const QMimeData& data) {
  auto encodedData = data.data(QString::fromStdString(CanvasNode::MIME_TYPE));
  vector<unique_ptr<CanvasNode>> nodes;
  if(encodedData.isEmpty()) {
    return nodes;
  }
  SharedBuffer buffer(encodedData.data(), encodedData.size());
  TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
  RegisterSpireTypes(Store(typeRegistry));
  auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
  receiver.SetSource(Ref(buffer));
  receiver.Shuttle(nodes);
  return nodes;
}

unique_ptr<CanvasNode> Spire::Convert(unique_ptr<CanvasNode> node,
    const CanvasType& type) {
  if(IsCompatible(type, node->GetType())) {
    return node;
  } else if(dynamic_cast<const NoneNode*>(node.get())) {
    return MakeDefaultCanvasNode(type);
  }
  return node->Convert(type);
}

unique_ptr<CanvasNode> Spire::ForceConversion(unique_ptr<CanvasNode> node,
    const CanvasType& type) {
  if(IsCompatible(type, node->GetType())) {
    return node;
  }
  if(dynamic_cast<const NoneNode*>(node.get())) {
    return MakeDefaultCanvasNode(type);
  }
  try {
    return node->Convert(type);
  } catch(const CanvasOperationException&) {
    return MakeDefaultCanvasNode(type);
  }
}

bool Spire::CheckClipboardForCanvasNode() {
  return !QApplication::clipboard()->mimeData()->data(
    QString::fromStdString(CanvasNode::MIME_TYPE)).isEmpty();
}

CanvasNodeNameSuffix Spire::SplitName(const std::string& name) {
  CanvasNodeNameSuffix splitName;
  static const boost::regex splitter("[^\\\\]\\.");
  boost::smatch matcher;
  if(!regex_search(name, matcher, splitter)) {
    splitName.m_identifier = name;
  } else {
    splitName.m_identifier = name.substr(0, matcher[0].first -
      name.begin() + 1);
    splitName.m_suffix = name.substr(matcher[0].second - name.begin());
  }
  return splitName;
}

string Spire::GetFullName(const CanvasNode& node) {
  if(IsRoot(node) || IsRoot(*node.GetParent())) {
    return node.GetName();
  }
  return GetFullName(*node.GetParent()) + "." + node.GetName();
}

int Spire::GetHeight(const CanvasNode& from, const CanvasNode& to) {
  if(&to == &from) {
    return 0;
  }
  if(!IsParent(from, to)) {
    return -GetHeight(to, from);
  }
  auto height = 1;
  auto i = to.GetParent();
  while(i.is_initialized() && &*i != &from) {
    ++height;
    i = i->GetParent();
  }
  return height;
}

string Spire::GetPath(const CanvasNode& source, const CanvasNode& destination) {
  assert(&GetRoot(source) == &GetRoot(destination));
  if(&source == &destination) {
    return "";
  }
  auto& commonAncestor = GetCommonAncestor(source, destination);
  string prefix;
  if(&commonAncestor != &source) {
    auto i = source.GetParent();
    while(i.is_initialized() && &*i != &commonAncestor) {
      prefix += "<";
      i = i->GetParent();
    }
  }
  string suffix;
  boost::optional<const CanvasNode&> i = destination;
  if(&destination == &commonAncestor) {
    return prefix + "<";
  }
  while(i.is_initialized() && &*i != &commonAncestor) {
    if(suffix.empty()) {
      suffix = i->GetName();
    } else {
      suffix = i->GetName() + "." + suffix;
    }
    i = i->GetParent();
  }
  return prefix + suffix;
}

string Spire::AppendCanvasNodePaths(const string& prefix,
    const string& suffix) {
  return prefix + suffix;
}

bool Spire::IsSame(boost::optional<const CanvasNode&> a,
    boost::optional<const CanvasNode&> b) {
  if(a.is_initialized() && b.is_initialized()) {
    return &(*a) == &(*b);
  }
  return !a.is_initialized() && !b.is_initialized();
}

bool Spire::IsRoot(const CanvasNode& node) {
  return !node.GetParent().is_initialized();
}

const CanvasNode& Spire::GetRoot(const CanvasNode& node) {
  if(IsRoot(node)) {
    return node;
  }
  return GetRoot(*node.GetParent());
}

const CanvasNode& Spire::GetCommonAncestor(const CanvasNode& a,
    const CanvasNode& b) {
  boost::optional<const CanvasNode&> i = a;
  boost::optional<const CanvasNode&> j = b;
  auto aDepth = 0;
  auto bDepth = 0;
  while(i.is_initialized()) {
    i = i->GetParent();
    ++aDepth;
  }
  while(j.is_initialized()) {
    j = j->GetParent();
    ++bDepth;
  }
  i = a;
  j = b;
  if(aDepth > bDepth) {
    for(auto k = 0; k < aDepth - bDepth; ++k) {
      i = i->GetParent();
    }
  } else if(bDepth > aDepth) {
    for(auto k = 0; k < bDepth - aDepth; ++k) {
      j = j->GetParent();
    }
  }
  while(&*i != &*j) {
    i = i->GetParent();
    j = j->GetParent();
  }
  return *i;
}

bool Spire::IsParent(const CanvasNode& a, const CanvasNode& b) {
  auto parent = b.GetParent();
  if(!parent) {
    return false;
  } else if(&(*parent) == &a) {
    return true;
  }
  return IsParent(a, *parent);
}
