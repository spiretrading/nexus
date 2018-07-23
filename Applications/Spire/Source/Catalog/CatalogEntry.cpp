#include "Spire/Catalog/CatalogEntry.hpp"
#include <sstream>
#include <boost/variant/get.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <QMimeData>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Catalog/CatalogSettings.hpp"

using namespace boost;
using namespace boost::uuids;
using namespace Spire;
using namespace std;

const string CatalogEntry::MIME_TYPE = "application/x-spire.catalog_entry";

const string& CatalogEntry::GetCatalogEntryKey() {
  static string key = "spire.catalog_entry";
  return key;
}

const string& CatalogEntry::GetCatalogEntrySourceKey() {
  static string key = "spire.catalog_entry_source";
  return key;
}

QMimeData* CatalogEntry::EncodeAsMimeData(
    const vector<CatalogEntry*>& entries) {
  auto mimeData = new QMimeData();
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);
  for(const auto& entry : entries) {
    stringstream sink;
    sink << entry->GetUid();
    stream << QString::fromStdString(sink.str());
  }
  mimeData->setData(QString::fromStdString(CatalogEntry::MIME_TYPE),
    encodedData);
  return mimeData;
}

vector<CatalogEntry*> CatalogEntry::DecodeFromMimeData(const QMimeData& data,
    const CatalogSettings& settings) {
  auto encodedData = data.data(QString::fromStdString(CatalogEntry::MIME_TYPE));
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  vector<CatalogEntry*> entries;
  QStringList items;
  string_generator stringGenerator;
  while(!stream.atEnd()) {
    QString data;
    stream >> data;
    auto source = data.toStdString();
    uuid entryId;
    boost::optional<CatalogEntry&> entry;
    try {
      entryId = stringGenerator(source);
      entry = settings.FindEntry(entryId);
    } catch(std::exception&) {}
    if(entry) {
      entries.push_back(&*entry);
    }
  }
  return entries;
}

boost::optional<uuid> CatalogEntry::FindUuid(const CanvasNode& node) {
  auto catalogMetaData = node.FindMetaData(CatalogEntry::GetCatalogEntryKey());
  if(!catalogMetaData.is_initialized()) {
    return none;
  }
  auto value = get<const string>(&*catalogMetaData);
  if(value == nullptr) {
    return none;
  }
  string_generator stringUuidGenerator;
  uuid catalogUuid;
  try {
    catalogUuid = stringUuidGenerator(*value);
  } catch(std::exception&) {
    return none;
  }
  return catalogUuid;
}

const uuid& CatalogEntry::GetUid() const {
  return m_uuid;
}

CatalogEntry::CatalogEntry(const uuid& uid)
    : m_uuid(uid) {}

void CatalogEntry::SetDescription(const string& description) {}
