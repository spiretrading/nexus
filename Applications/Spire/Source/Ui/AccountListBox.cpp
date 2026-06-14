#include "Spire/Ui/AccountListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Spire;

AccountListBox* Spire::make_account_list_box(
    std::shared_ptr<AccountQueryModel> accounts, QWidget* parent) {
  return make_account_list_box(std::move(accounts),
    std::make_shared<ArrayListModel<AccountListItem::Account>>(), parent);
}

AccountListBox* Spire::make_account_list_box(
    std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent) {
  return new AccountListBox(std::move(accounts), std::move(current),
    [] (const std::shared_ptr<AccountListModel>& list, int index) {
      return new AccountListItem(list->get(index));
    }, parent);
}
