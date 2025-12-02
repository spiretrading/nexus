#include "Spire/AccountViewer/ServiceItem.hpp"
#include <QIcon>

using namespace Beam;
using namespace Spire;
using namespace std;

ServiceItem::ServiceItem(const DirectoryEntry& entry)
  : m_entry(entry) {}

ServiceItem::~ServiceItem() {}

const DirectoryEntry& ServiceItem::GetEntry() const {
  return m_entry;
}

ServiceItem::Type ServiceItem::GetType() const {
  return Type::SERVICE;
}

string ServiceItem::GetName() const {
  return m_entry.m_name;
}

QIcon ServiceItem::GetIcon() const {
  return QIcon(":/icons/server_small.png");
}
