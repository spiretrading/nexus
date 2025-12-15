#include "Spire/AccountViewer/ManagerItem.hpp"
#include <QIcon>

using namespace Beam;
using namespace Spire;
using namespace std;

ManagerItem::ManagerItem(const DirectoryEntry& entry)
  : m_entry(entry) {}

ManagerItem::~ManagerItem() {}

const DirectoryEntry& ManagerItem::GetEntry() const {
  return m_entry;
}

ManagerItem::Type ManagerItem::GetType() const {
  return Type::MANAGER;
}

string ManagerItem::GetName() const {
  return m_entry.m_name;
}

QIcon ManagerItem::GetIcon() const {
  return QIcon(":/icons/stockbroker2_small.png");
}
