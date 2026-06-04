#include "Spire/Ui/AccountFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListModel.hpp"

using namespace boost::signals2;
using namespace Spire;

bool OpenFilterPanelAdaptor<AccountListBox>::is_empty(
    AccountListBox& account_list_box) {
  return account_list_box.get_current()->get_size() == 0;
}

void OpenFilterPanelAdaptor<AccountListBox>::clear(
    AccountListBox& account_list_box) {
  Spire::clear(*account_list_box.get_current());
}

const OpenFilterPanelAdaptor<AccountListBox>::Type&
    OpenFilterPanelAdaptor<AccountListBox>::get_current(
      AccountListBox& account_list_box) {
  return account_list_box.get_current();
}

connection OpenFilterPanelAdaptor<AccountListBox>::connect_current(
    AccountListBox& account_list_box, const std::function<void()>& slot) {
  return account_list_box.get_current()->connect_operation_signal(
    [=] (const auto& operation) {
      visit(operation,
        [&] (const AccountListModel::AddOperation&) {
          slot();
        },
        [&] (const AccountListModel::RemoveOperation&) {
          slot();
        });
    });
}

template class OpenFilterPanel<AccountListBox>;

AccountFilterPanel* Spire::make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts, QWidget* parent) {
  return make_account_filter_panel(std::move(accounts),
    std::make_shared<ArrayListModel<AccountListItem::Account>>(), parent);
}

AccountFilterPanel* Spire::make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent) {
  auto box = new AccountListBox(std::move(accounts), std::move(current));
  box->set_placeholder(QObject::tr("Enter accounts"));
  return new OpenFilterPanel(*box, parent);
}
