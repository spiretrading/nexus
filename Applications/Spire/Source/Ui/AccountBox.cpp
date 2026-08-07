#include "Spire/Ui/AccountBox.hpp"
#include <unordered_set>
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

struct AccountBox::AccountToAccountIdQueryModel : QueryModel<QString> {
  std::shared_ptr<AccountQueryModel> m_source;

  explicit AccountToAccountIdQueryModel(
    std::shared_ptr<AccountQueryModel> source)
    : m_source(std::move(source)) {}

  optional<QString> parse(const QString& query) override {
    auto account = m_source->parse(query);
    if(account && QString::fromStdString(account->m_account.m_name).toLower() ==
        query.toLower()) {
      return QString::fromStdString(account->m_account.m_name);
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
          auto id = QString::fromStdString(match.m_account.m_name);
          if(seen.insert(id).second) {
            ids.push_back(id);
          }
        }
        return ids;
      });
  }
};

AccountBox::AccountBox(
  std::shared_ptr<AccountQueryModel> accounts, QWidget* parent)
  : AccountBox(
      std::move(accounts), std::make_shared<LocalAccountModel>(), parent) {}

AccountBox::AccountBox(std::shared_ptr<AccountQueryModel> accounts,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_accounts(
        std::make_shared<AccountToAccountIdQueryModel>(std::move(accounts))),
      m_current(std::move(current)),
      m_submission(m_current->get()) {
  auto id_model = make_transform_value_model(m_current,
    [] (const AccountListItem::Account& account) {
      return QString::fromStdString(account.m_account.m_name);
    },
    [=] (const QString& id) {
      if(auto account = m_accounts->m_source->parse(id)) {
        return *account;
      }
      throw std::invalid_argument("Invalid account id.");
    });
  m_combo_box = new ComboBox<QString>(m_accounts, id_model,
    [=] (const auto& list, auto index) {
      if(auto account = m_accounts->m_source->parse(list->get(index))) {
        return new AccountListItem(*account);
      }
      return new AccountListItem(AccountListItem::Account());
    });
  m_submit_connection = m_combo_box->connect_submit_signal(
    [=] (const auto& id) {
      if(auto account = m_accounts->m_source->parse(id)) {
        m_submission = *account;
        m_submit_signal(m_submission);
      }
    });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<AccountQueryModel>& AccountBox::get_accounts() const {
  return m_accounts->m_source;
}

const std::shared_ptr<AccountBox::CurrentModel>&
    AccountBox::get_current() const {
  return m_current;
}

const AccountListItem::Account& AccountBox::get_submission() const {
  return m_submission;
}

void AccountBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool AccountBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void AccountBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection AccountBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
