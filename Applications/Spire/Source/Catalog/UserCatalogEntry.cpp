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
#include "Spire/LegacyUI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace boost::uuids;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  auto GenerateUid() {
    static auto generator = random_generator();
    return generator();
  }
}

const std::string& UserCatalogEntry::GetUserSourceValue() {
  static auto value = std::string("UserCatalogEntry");
  return value;
}

UserCatalogEntry::UserCatalogEntry(const std::filesystem::path& catalogPath)
  : m_catalogPath(catalogPath) {}

UserCatalogEntry::UserCatalogEntry(const std::string& name,
  const CanvasNode& node, const std::string& iconPath,
  const std::string& description, const std::filesystem::path& catalogPath)
  : UserCatalogEntry(
      name, node, GenerateUid(), iconPath, description, catalogPath) {}

UserCatalogEntry::UserCatalogEntry(const std::string& name,
    const CanvasNode& node, const uuid& uuid, const std::string& iconPath,
    const std::string& description, const std::filesystem::path& catalogPath)
    : PersistentCatalogEntry(uuid),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QIcon(QString::fromStdString(m_iconPath))),
      m_description(description),
      m_catalogPath(catalogPath) {
  auto builder = CanvasNodeBuilder(node);
  auto ss = std::stringstream();
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    UserCatalogEntry::GetUserSourceValue());
  m_node = builder.Make();
}

void UserCatalogEntry::Save() const {
  auto entryPath = m_catalogPath / (m_name + ".cat");
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(*this);
    auto writer = BasicOStreamWriter<std::ofstream>(
      Initialize(entryPath, std::ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save catalog entry."));
  }
}

void UserCatalogEntry::Delete() const {
  auto entryPath = m_catalogPath / (m_name + ".cat");
  try {
    std::filesystem::remove(entryPath);
  } catch(const std::exception&) {}
}

std::string UserCatalogEntry::GetName() const {
  return m_name;
}

std::unique_ptr<CatalogEntry>
    UserCatalogEntry::SetName(const std::string& name) const {
  return std::make_unique<UserCatalogEntry>(
    name, GetNode(), GetUid(), GetIconPath(), GetDescription(), m_catalogPath);
}

const std::string& UserCatalogEntry::GetIconPath() const {
  return m_iconPath;
}

std::unique_ptr<CatalogEntry>
    UserCatalogEntry::SetIconPath(const std::string& iconPath) const {
  return std::make_unique<UserCatalogEntry>(
    GetName(), GetNode(), GetUid(), iconPath, GetDescription(), m_catalogPath);
}

QIcon UserCatalogEntry::GetIcon() const {
  return m_icon;
}

std::string UserCatalogEntry::GetDescription() const {
  return m_description;
}

const CanvasNode& UserCatalogEntry::GetNode() const {
  return *m_node;
}

std::unique_ptr<CatalogEntry>
    UserCatalogEntry::SetNode(const CanvasNode& node) const {
  return std::make_unique<UserCatalogEntry>(
    GetName(), node, GetUid(), GetIconPath(), GetDescription(), m_catalogPath);
}

bool UserCatalogEntry::IsReadOnly() const {
  return false;
}

void UserCatalogEntry::Validate() {
  auto catalogUuid = FindUuid(*m_node);
  if(!catalogUuid || *catalogUuid != GetUid()) {
    auto builder = CanvasNodeBuilder(*m_node);
    auto ss = std::stringstream();
    ss << GetUid();
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntryKey(), ss.str());
    builder.SetMetaData(*m_node, CatalogEntry::GetCatalogEntrySourceKey(),
      UserCatalogEntry::GetUserSourceValue());
    m_node = builder.Make();
  }
}
