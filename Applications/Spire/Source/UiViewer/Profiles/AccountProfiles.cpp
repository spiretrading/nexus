#include "Spire/UiViewer/Profiles/AccountProfiles.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/AccountBox.hpp"
#include "Spire/Ui/AccountFilterPanel.hpp"
#include "Spire/Ui/AccountListBox.hpp"
#include "Spire/Ui/AccountListItem.hpp"
#include "Spire/Ui/Identicon.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

namespace {
  std::shared_ptr<AccountQueryModel> populate_account_query_model() {
    auto accounts = std::vector<std::pair<QString, QString>>{
      {"meixiangk20", "Kong Meixiang"}, {"mingzhuca11", "Cao Mingzhu"},
      {"hengshen08", "Shen Heng"}, {"guozhido02", "Dong Guozhi"},
      {"huanxg34", "Xu Guanghuan"}, {"fengjg15", "Jiang Feng"}};
    auto model = std::make_shared<LocalQueryModel<AccountListItem::Account>>();
    for(auto& [id, name] : accounts) {
      auto entry = DirectoryEntry::make_account(
        qHash(id), id.toStdString());
      auto account = AccountListItem::Account(
        make_identicon(entry, scale(8, 8)), entry, name);
      model->add(id.toLower(), account);
      for(auto& term : name.split(' ')) {
        model->add(term.toLower(), account);
      }
    }
    return model;
  }
}

UiProfile Spire::make_account_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("current", "meixiangk20"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("AccountBox", properties, [] (auto& profile) {
    auto to_id = [] (const AccountListItem::Account& account) {
      return QString::fromStdString(account.m_account.m_name);
    };
    auto model = populate_account_query_model();
    auto& current = get<QString>("current", profile.get_properties());
    auto current_account = [&] {
      if(auto value = model->parse(current.get())) {
        return *value;
      }
      return AccountListItem::Account();
    }();
    auto current_model = std::make_shared<LocalAccountModel>(current_account);
    auto box = new AccountBox(model, current_model);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<AccountListItem::Account>("Current", to_id));
    current.connect_changed_signal([=] (const auto& current) {
      if(auto value = model->parse(current)) {
        box->get_current()->set(*value);
      } else {
        auto current_blocker = shared_connection_block(current_connection);
        box->get_current()->set(AccountListItem::Account());
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&AccountBox::set_read_only, box));
    box->connect_submit_signal(
      profile.make_event_slot<AccountListItem::Account>("Submit", to_id));
    return box;
  });
  return profile;
}

UiProfile Spire::make_account_filter_panel_profile() {
  return setup_open_filter_panel_profile("AccountFilterPanel",
    [] { return make_account_filter_panel(populate_account_query_model()); });
}

UiProfile Spire::make_account_list_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  return UiProfile("AccountListBox", properties, [] (auto& profile) {
    auto box = make_account_list_box(populate_account_query_model());
    box->setMinimumWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&AccountListBox::set_read_only, box));
    auto current_slot = profile.make_event_slot<QString>("Current");
    auto print_current = [=] {
      auto result = QString();
      for(auto i = 0; i < box->get_current()->get_size(); ++i) {
        result += QString("[%1] ").arg(
          QString::fromStdString(box->get_current()->get(i).m_account.m_name));
      }
      current_slot(result);
    };
    box->get_current()->connect_operation_signal(
      [=] (const auto& operation) {
        visit(operation,
          [=] (const AccountListModel::AddOperation&) {
            print_current();
          },
          [=] (const AccountListModel::RemoveOperation&) {
            print_current();
          });
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    box->connect_submit_signal([=] (const auto& submission) {
      auto result = QString();
      for(auto i = 0; i < submission->get_size(); ++i) {
        result += QString("[%1] ").arg(
          QString::fromStdString(submission->get(i).m_account.m_name));
      }
      submit_slot(result);
    });
    return box;
  });
}

UiProfile Spire::make_account_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("id", "meixiangk20"));
  properties.push_back(
    make_standard_property<QString>("name", "Kong Meixiang"));
  auto profile = UiProfile("AccountListItem", properties,
    [] (auto& profile) {
      auto& id = get<QString>("id", profile.get_properties());
      auto& name = get<QString>("name", profile.get_properties());
      auto entry = DirectoryEntry::make_account(
        qHash(id.get()), id.get().toStdString());
      auto identicon = make_identicon(entry, scale(8, 8));
      auto item = new AccountListItem(
        AccountListItem::Account(identicon, entry, name.get()));
      apply_widget_properties(item, profile.get_properties());
      return item;
    });
  return profile;
}
