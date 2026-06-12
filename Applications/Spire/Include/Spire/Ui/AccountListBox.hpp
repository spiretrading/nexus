#ifndef SPIRE_ACCOUNT_LIST_BOX_HPP
#define SPIRE_ACCOUNT_LIST_BOX_HPP
#include "Spire/Ui/AccountBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** Represents a ListModel for a list of AccountListItem::Account values. */
  using AccountListModel = ListModel<AccountListItem::Account>;

  /** A TagComboBox specialized for an AccountListItem::Account. */
  using AccountListBox = TagComboBox<AccountListItem::Account>;

  /**
   * Returns a new AccountListBox using a default current model.
   * @param accounts The set of accounts that can be queried.
   * @param parent The parent widget.
   */
  AccountListBox* make_account_list_box(
    std::shared_ptr<AccountQueryModel> accounts, QWidget* parent = nullptr);

  /**
   * Returns a new AccountListBox.
   * @param accounts The set of accounts that can be queried.
   * @param current The current list of selected accounts.
   * @param parent The parent widget.
   */
  AccountListBox* make_account_list_box(
    std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent = nullptr);
}

#endif
