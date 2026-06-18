#include "Spire/Ui/AccountFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Spire;

template class OpenFilterPanel<AccountListBox>;

AccountFilterPanel* Spire::make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts, QWidget* parent) {
  return make_account_filter_panel(std::move(accounts),
    std::make_shared<ArrayListModel<AccountListItem::Account>>(), parent);
}

AccountFilterPanel* Spire::make_account_filter_panel(
    std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent) {
  auto box = make_account_list_box(std::move(accounts), std::move(current));
  box->set_placeholder(QObject::tr("Enter accounts"));
  return new OpenFilterPanel(*box, parent);
}
