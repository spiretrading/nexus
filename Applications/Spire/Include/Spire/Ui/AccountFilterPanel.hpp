#ifndef SPIRE_ACCOUNT_FILTER_PANEL_HPP
#define SPIRE_ACCOUNT_FILTER_PANEL_HPP
#include "Spire/Ui/AccountListBox.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"

namespace Spire {

  extern template class OpenFilterPanel<AccountListBox>;

  /** An OpenFilterPanel specialized for an AccountListBox. */
  using AccountFilterPanel = OpenFilterPanel<AccountListBox>;

  /**
   * Returns a new AccountFilterPanel with a default current model.
   * @param accounts The set of accounts that can be queried.
   * @param parent The parent widget.
   */
  AccountFilterPanel* make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts, QWidget* parent = nullptr);

  /**
   * Returns a new AccountFilterPanel.
   * @param accounts The set of accounts that can be queried.
   * @param current The current list of selected accounts.
   * @param parent The parent widget.
   */
  AccountFilterPanel* make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent = nullptr);
}

#endif
