#ifndef SPIRE_ACCOUNT_FILTER_PANEL_HPP
#define SPIRE_ACCOUNT_FILTER_PANEL_HPP
#include "Spire/Ui/AccountListBox.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"

namespace Spire {

  template<>
  struct OpenFilterPanelAdaptor<AccountListBox> {
    using Type = std::shared_ptr<AccountListModel>;

    static bool is_empty(AccountListBox& account_list_box);
    static void clear(AccountListBox& account_list_box);
    static const Type& get_current(AccountListBox& account_list_box);
    static boost::signals2::connection connect_current(
      AccountListBox& account_list_box, const std::function<void()>& slot);
  };

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
