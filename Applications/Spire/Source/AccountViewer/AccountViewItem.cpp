#include "Spire/AccountViewer/AccountViewItem.hpp"

using namespace Spire;
using namespace std;

AccountViewItem::~AccountViewItem() {}

vector<unique_ptr<AccountViewItem>> AccountViewItem::LoadChildren(
    const UserProfile& userProfile) const {
  return vector<unique_ptr<AccountViewItem>>();
}
