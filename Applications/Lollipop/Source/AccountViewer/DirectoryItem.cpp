#include "Spire/AccountViewer/DirectoryItem.hpp"
#include <QIcon>
#include "Spire/AccountViewer/ManagerItem.hpp"
#include "Spire/AccountViewer/TraderItem.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

DirectoryItem::DirectoryItem(const DirectoryEntry& entry, DirectoryType type)
  : m_entry(entry),
    m_directoryType(type) {}

DirectoryItem::~DirectoryItem() {}

const DirectoryEntry& DirectoryItem::GetEntry() const {
  return m_entry;
}

DirectoryItem::DirectoryType DirectoryItem::GetDirectoryType() const {
  return m_directoryType;
}

DirectoryItem::Type DirectoryItem::GetType() const {
  return Type::DIRECTORY;
}

string DirectoryItem::GetName() const {
  return m_entry.m_name;
}

QIcon DirectoryItem::GetIcon() const {
  return QIcon(":/icons/folder2_yellow_small.png");
}

vector<unique_ptr<AccountViewItem>> DirectoryItem::LoadChildren(
    const UserProfile& userProfile) const {
  auto children =
    userProfile.GetClients().get_service_locator_client().load_children(
      m_entry);
  std::sort(children.begin(), children.end(), DirectoryEntry::name_comparator);
  vector<unique_ptr<AccountViewItem>> childrenItems;
  for(auto& child : children) {
    if(child.m_type == DirectoryEntry::Type::ACCOUNT) {
      if(m_entry.m_name == "managers") {
        auto manager = std::make_unique<ManagerItem>(child);
        childrenItems.push_back(std::move(manager));
      } else if(m_entry.m_name == "traders") {
        auto trader = std::make_unique<TraderItem>(child);
        childrenItems.push_back(std::move(trader));
      }
    } else {
      auto type = [&] {
        if(child.m_name == "managers") {
          return DirectoryItem::DirectoryType::MANAGERS;
        } else if(child.m_name == "traders") {
          return DirectoryItem::DirectoryType::TRADERS;
        } else {
          return DirectoryItem::DirectoryType::OTHER;
        }
      }();
      auto directory = std::make_unique<DirectoryItem>(child, type);
      childrenItems.push_back(std::move(directory));
    }
  }
  return childrenItems;
}
