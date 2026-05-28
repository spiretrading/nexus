#include "Spire/Ui/AccountListBox.hpp"
#include <unordered_set>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TransformListModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TagComboBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

struct AccountListBox::AccountToAccountIdQueryModel : QueryModel<QString> {
  std::shared_ptr<AccountQueryModel> m_source;

  explicit AccountToAccountIdQueryModel(
    std::shared_ptr<AccountQueryModel> source)
    : m_source(std::move(source)) {}

  optional<QString> parse(const QString& query) override {
    if(auto account = m_source->parse(query); account &&
        QString::compare(account->m_id, query, Qt::CaseInsensitive) == 0) {
      return account->m_id;
    }
    return none;
  }

  QtPromise<std::vector<QString>> submit(const QString& query) override {
    return m_source->submit(query).then(
      [] (const std::vector<AccountListItem::Account>& matches) {
        auto seen = std::unordered_set<QString>();
        auto ids = std::vector<QString>();
        ids.reserve(matches.size());
        for(auto& match : matches) {
          if(seen.insert(match.m_id).second) {
            ids.push_back(match.m_id);
          }
        }
        return ids;
      });
  }
};

AccountListBox::AccountListBox(
  std::shared_ptr<AccountQueryModel> accounts, QWidget* parent)
  : AccountListBox(std::move(accounts),
      std::make_shared<ArrayListModel<AccountListItem::Account>>(), parent) {}

AccountListBox::AccountListBox(std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<AccountListModel> current, QWidget* parent)
    : QWidget(parent),
      m_accounts(std::make_shared<AccountToAccountIdQueryModel>(
        std::move(accounts))),
      m_current(std::move(current)) {
  auto to_id = [] (const AccountListItem::Account& account) {
    return account.m_id;
  };
  auto from_id = [=] (const QString& id) {
    if(auto account = m_accounts->m_source->parse(id)) {
      return *account;
    }
    throw std::invalid_argument("Invalid account id.");
  };
  using IdListModel = decltype(TransformListModel(m_current, to_id, from_id));
  m_tag_combo_box = new TagComboBox<QString>(m_accounts,
    std::make_shared<IdListModel>(m_current, to_id, from_id),
    [=] (const auto& list, auto index) {
      if(auto account = m_accounts->m_source->parse(list->get(index))) {
        return new AccountListItem(*account);
      }
      return new AccountListItem(AccountListItem::Account());
    });
  m_submit_connection = m_tag_combo_box->connect_submit_signal(
    [=] (const std::shared_ptr<ListModel<QString>>& submission) {
      auto submitted_accounts =
        std::make_shared<ArrayListModel<AccountListItem::Account>>();
      for(auto i = 0; i < submission->get_size(); ++i) {
        if(auto account = m_accounts->m_source->parse(submission->get(i))) {
          submitted_accounts->push(*account);
        }
      }
      m_submit_signal(submitted_accounts);
    });
  enclose(*this, *m_tag_combo_box);
  proxy_style(*this, *m_tag_combo_box);
  setFocusProxy(m_tag_combo_box);
}

const std::shared_ptr<AccountQueryModel>& AccountListBox::get_accounts() const {
  return m_accounts->m_source;
}

const std::shared_ptr<AccountListModel>& AccountListBox::get_current() const {
  return m_current;
}

void AccountListBox::set_placeholder(const QString& placeholder) {
  m_tag_combo_box->set_placeholder(placeholder);
}

bool AccountListBox::is_read_only() const {
  return m_tag_combo_box->is_read_only();
}

void AccountListBox::set_read_only(bool is_read_only) {
  m_tag_combo_box->set_read_only(is_read_only);
}

connection AccountListBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
