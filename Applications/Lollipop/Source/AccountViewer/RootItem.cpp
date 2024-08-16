#include "Spire/AccountViewer/RootItem.hpp"
#include <QIcon>

using namespace Spire;
using namespace std;

RootItem::RootItem(const string& name)
    : m_name(name) {}

RootItem::~RootItem() {}

RootItem::Type RootItem::GetType() const {
  return Type::ROOT;
}

string RootItem::GetName() const {
  return m_name;
}

QIcon RootItem::GetIcon() const {
  return QIcon(":/icons/folder2_yellow_small.png");
}
