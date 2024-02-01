#include "Spire/Catalog/RegistryCatalogEntry.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Utilities/NotSupportedException.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::RegistryService;
using namespace Beam::Serialization;
using namespace boost;
using namespace boost::uuids;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  uuid GenerateUid() {
    static random_generator generator;
    return generator();
  }
}

const string& RegistryCatalogEntry::GetRegistrySourceValue() {
  static string value = "RegistryCatalogEntry";
  return value;
}

RegistryCatalogEntry::RegistryCatalogEntry(bool isReadOnly,
    const string& path, RegistryClientBox registryClient)
    : m_isReadOnly(isReadOnly),
      m_path(path),
      m_registryClient(std::move(registryClient)) {}

RegistryCatalogEntry::RegistryCatalogEntry(const string& name,
    const string& iconPath, const string& description, const CanvasNode& node,
    const string& path, RegistryClientBox registryClient)
    : PersistentCatalogEntry(GenerateUid()),
      m_isReadOnly(false),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QString::fromStdString(iconPath)),
      m_description(description),
      m_path(path),
      m_registryClient(std::move(registryClient)) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    RegistryCatalogEntry::GetRegistrySourceValue());
  m_node = builder.Make();
}

RegistryCatalogEntry::RegistryCatalogEntry(const string& name,
    const string& iconPath, const string& description, const CanvasNode& node,
    const uuid& uid, const string& path, RegistryClientBox registryClient)
    : PersistentCatalogEntry(uid),
      m_isReadOnly(false),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QString::fromStdString(iconPath)),
      m_description(description),
      m_path(path),
      m_registryClient(std::move(registryClient)) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    RegistryCatalogEntry::GetRegistrySourceValue());
  m_node = builder.Make();
}

void RegistryCatalogEntry::Save() const {
  string path = m_path + "/" + GetName();
  RegistryEntry directory = LoadOrCreateDirectory(m_registryClient, m_path,
    RegistryEntry::GetRoot());
  SharedBuffer buffer;
  TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
  RegisterSpireTypes(Store(typeRegistry));
  auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
  sender.SetSink(Ref(buffer));
  sender.Shuttle(*this);
  m_registryClient.Store(GetName(), buffer, directory);
}

void RegistryCatalogEntry::Delete() const {
  try {
    string path = m_path + "/" + GetName();
    RegistryEntry entry = m_registryClient.LoadPath(
      RegistryEntry::GetRoot(), path);
    m_registryClient.Delete(entry);
  } catch(std::exception&) {}
}

string RegistryCatalogEntry::GetName() const {
  return m_name;
}

unique_ptr<CatalogEntry> RegistryCatalogEntry::SetName(
    const string& name) const {
  if(!m_isReadOnly) {
    BOOST_THROW_EXCEPTION(NotSupportedException(
      "RegistryCatalogEntry::SetName"));
  }
  unique_ptr<CatalogEntry> entry = std::make_unique<RegistryCatalogEntry>(name,
    GetIconPath(), GetDescription(), GetNode(), GetUid(), m_path,
    m_registryClient);
  return entry;
}

const string& RegistryCatalogEntry::GetIconPath() const {
  return m_iconPath;
}

unique_ptr<CatalogEntry> RegistryCatalogEntry::SetIconPath(
    const string& iconPath) const {
  if(!m_isReadOnly) {
    BOOST_THROW_EXCEPTION(NotSupportedException(
      "RegistryCatalogEntry::SetIconPath"));
  }
  unique_ptr<CatalogEntry> entry = std::make_unique<RegistryCatalogEntry>(
    GetName(), iconPath, GetDescription(), GetNode(), GetUid(), m_path,
    m_registryClient);
  return entry;
}

QIcon RegistryCatalogEntry::GetIcon() const {
  return m_icon;
}

string RegistryCatalogEntry::GetDescription() const {
  return m_description;
}

const CanvasNode& RegistryCatalogEntry::GetNode() const {
  return *m_node;
}

unique_ptr<CatalogEntry> RegistryCatalogEntry::SetNode(
    const CanvasNode& node) const {
  if(!m_isReadOnly) {
    BOOST_THROW_EXCEPTION(NotSupportedException(
      "RegistryCatalogEntry::SetNode"));
  }
  unique_ptr<CatalogEntry> entry = std::make_unique<RegistryCatalogEntry>(
    GetName(), GetIconPath(), GetDescription(), node, GetUid(), m_path,
    m_registryClient);
  return entry;
}

bool RegistryCatalogEntry::IsReadOnly() const {
  return m_isReadOnly;
}

void RegistryCatalogEntry::Validate() {
  auto catalogUuid = FindUuid(*m_node);
  if(!catalogUuid.is_initialized() || *catalogUuid != GetUid()) {
    CanvasNodeBuilder builder(*m_node);
    stringstream ss;
    ss << GetUid();
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntryKey(), ss.str());
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntrySourceKey(),
      RegistryCatalogEntry::GetRegistrySourceValue());
    m_node = builder.Make();
  }
}
