#include "Spire/Catalog/UserCatalogEntry.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <QMessageBox>
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/UI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::uuids;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

namespace {
  uuid GenerateUid() {
    static random_generator generator;
    return generator();
  }
}

const string& UserCatalogEntry::GetUserSourceValue() {
  static string value = "UserCatalogEntry";
  return value;
}

UserCatalogEntry::UserCatalogEntry(const path& catalogPath)
    : m_catalogPath(catalogPath) {}

UserCatalogEntry::UserCatalogEntry(const string& name, const CanvasNode& node,
    const string& iconPath, const string& description, const path& catalogPath)
    : PersistentCatalogEntry(GenerateUid()),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QIcon(QString::fromStdString(m_iconPath))),
      m_description(description),
      m_catalogPath(catalogPath) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    UserCatalogEntry::GetUserSourceValue());
  m_node = builder.Make();
}

UserCatalogEntry::UserCatalogEntry(const string& name, const CanvasNode& node,
    const uuid& uuid, const string& iconPath, const string& description,
    const path& catalogPath)
    : PersistentCatalogEntry(uuid),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QIcon(QString::fromStdString(m_iconPath))),
      m_description(description),
      m_catalogPath(catalogPath) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    UserCatalogEntry::GetUserSourceValue());
  m_node = builder.Make();
}

UserCatalogEntry::~UserCatalogEntry() {}

void UserCatalogEntry::Save() const {
  path entryPath = m_catalogPath / (m_name + ".cat");
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(*this);
    BasicOStreamWriter<ofstream> writer(Initialize(entryPath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save catalog entry."));
  }
}

void UserCatalogEntry::Delete() const {
  path entryPath = m_catalogPath / (m_name + ".cat");
  try {
    remove(entryPath);
  } catch(std::exception&) {}
}

string UserCatalogEntry::GetName() const {
  return m_name;
}

unique_ptr<CatalogEntry> UserCatalogEntry::SetName(
    const string& name) const {
  unique_ptr<CatalogEntry> entry = std::make_unique<UserCatalogEntry>(name,
    GetNode(), GetUid(), GetIconPath(), GetDescription(), m_catalogPath);
  return entry;
}

const string& UserCatalogEntry::GetIconPath() const {
  return m_iconPath;
}

unique_ptr<CatalogEntry> UserCatalogEntry::SetIconPath(
    const string& iconPath) const {
  unique_ptr<CatalogEntry> entry = std::make_unique<UserCatalogEntry>(GetName(),
    GetNode(), GetUid(), iconPath, GetDescription(), m_catalogPath);
  return entry;
}

QIcon UserCatalogEntry::GetIcon() const {
  return m_icon;
}

string UserCatalogEntry::GetDescription() const {
  return m_description;
}

const CanvasNode& UserCatalogEntry::GetNode() const {
  return *m_node;
}

unique_ptr<CatalogEntry> UserCatalogEntry::SetNode(
    const CanvasNode& node) const {
  unique_ptr<CatalogEntry> entry = std::make_unique<UserCatalogEntry>(GetName(),
    node, GetUid(), GetIconPath(), GetDescription(), m_catalogPath);
  return entry;
}

bool UserCatalogEntry::IsReadOnly() const {
  return false;
}

void UserCatalogEntry::Validate() {
  auto catalogUuid = FindUuid(*m_node);
  if(!catalogUuid.is_initialized() || *catalogUuid != GetUid()) {
    CanvasNodeBuilder builder(*m_node);
    stringstream ss;
    ss << GetUid();
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntryKey(), ss.str());
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntrySourceKey(),
      UserCatalogEntry::GetUserSourceValue());
    m_node = builder.Make();
  }
}
