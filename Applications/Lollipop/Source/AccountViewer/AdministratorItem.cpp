#include "Spire/AccountViewer/AdministratorItem.hpp"
#include <QIcon>

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;
using namespace std;

AdministratorItem::AdministratorItem(const DirectoryEntry& entry)
    : m_entry(entry) {}

AdministratorItem::~AdministratorItem() {}

const DirectoryEntry& AdministratorItem::GetEntry() const {
  return m_entry;
}

AdministratorItem::Type AdministratorItem::GetType() const {
  return Type::ADMINISTRATOR;
}

string AdministratorItem::GetName() const {
  return m_entry.m_name;
}

QIcon AdministratorItem::GetIcon() const {
  return QIcon(":/icons/chess_piece_small.png");
}
