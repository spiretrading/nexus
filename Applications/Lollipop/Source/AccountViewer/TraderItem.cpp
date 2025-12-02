#include "Spire/AccountViewer/TraderItem.hpp"
#include <QIcon>

using namespace Beam;
using namespace Spire;
using namespace std;

TraderItem::TraderItem(const DirectoryEntry& entry)
  : m_entry(entry) {}

TraderItem::~TraderItem() {}

const DirectoryEntry& TraderItem::GetEntry() const {
  return m_entry;
}

TraderItem::Type TraderItem::GetType() const {
  return Type::TRADER;
}

string TraderItem::GetName() const {
  return m_entry.m_name;
}

QIcon TraderItem::GetIcon() const {
  return QIcon(":/icons/pawn_glass_blue_small.png");
}
