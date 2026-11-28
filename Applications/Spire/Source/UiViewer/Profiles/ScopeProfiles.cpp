#include "Spire/UiViewer/Profiles/ScopeProfiles.hpp"
#include <QStringBuilder>
#include "Nexus/Definitions/StandardCountries.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ScopeBox.hpp"
#include "Spire/Ui/ScopeDropDownBox.hpp"
#include "Spire/Ui/ScopeFilterPanel.hpp"
#include "Spire/Ui/ScopeListItem.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  auto print_scope(const Scope& scope) {
    auto result = QString();
    result += "Scope{Countries{";
    for(auto& country : scope.get_countries()) {
      result += COUNTRIES.from(country).m_three_letter_code.get_data();
      result += " ";
    }
    result += "} Venues{";
    for(auto& venue : scope.get_venues()) {
      result += to_text(venue);
      result += " ";
    }
    result += "} Tickers{";
    for(auto& ticker : scope.get_tickers()) {
      result += to_text(ticker);
      result += " ";
    }
    result += "}}";
    return result;
  }
}

UiProfile Spire::make_scope_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("current", "TSX,USA,CAN"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("ScopeBox", properties, [] (auto& profile) {
    auto query_model = populate_scope_box_model();
    auto to_scope = [=] (const auto& text) {
      auto scope = Scope();
      for(auto& value : text.split(",")) {
        if(auto parse_value = query_model->parse(value)) {
          scope += *parse_value;
        }
      }
      return scope;
    };
    auto to_string = [] (const auto& scope) {
      auto text = QString();
      for(auto& country : scope.get_countries()) {
        text = text % to_text(country) % ",";
      }
      for(auto& venue : scope.get_venues()) {
        text = text % to_text(venue) % ",";
      }
      for(auto& ticker : scope.get_tickers()) {
        text = text % to_text(ticker) % ",";
      }
      text.remove(text.length() - 1, 1);
      return text;
    };
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model = make_local_value_model(to_scope(current.get()));
    current.connect_changed_signal([=] (const auto& value) {
      auto scope = to_scope(value);
      if(current_model->get() != scope) {
        current_model->set(scope);
      }
    });
    auto box = new ScopeBox(query_model, current_model);
    box->setMinimumWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ScopeBox::set_read_only, box));
    auto current_slot = profile.make_event_slot<QString>("Current");
    box->get_current()->connect_update_signal(
      [=, &current] (const Scope& scope) {
        current_slot(print_scope(scope));
        if(to_scope(current.get()) != scope) {
          current.set(to_string(scope));
        }
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    box->connect_submit_signal([=] (const Scope& scope) {
      submit_slot(print_scope(scope));
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_scope_drop_down_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_scope = define_enum<Scope>(
    {{"ASX", Venues::ASX}, {"CXD", Venues::CXD}, {"TSX", Venues::TSX},
     {"USA", Scope(Countries::US)}, {"CAN", Scope(Countries::CA)}});
  properties.push_back(make_standard_enum_property("current", current_scope));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("ScopeDropDownBox", properties, [] (auto& profile) {
    auto venues = std::vector{Venues::ASX, Venues::CXD, Venues::TSX};
    auto countries = std::vector{Countries::US, Countries::CA};
    auto scopes = std::make_shared<ArrayListModel<Scope>>();
    for(auto& venue : venues) {
      auto scope = Scope(VENUES.from(venue).m_display_name);
      scope += venue;
      scopes->push(scope);
    }
    for(auto& country : countries) {
      auto scope = Scope(COUNTRIES.from(country).m_name);
      scope += country;
      scopes->push(scope);
    }
    auto box = make_scope_drop_down_box(std::move(scopes));
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Scope>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ScopeDropDownBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<Scope>("Current"));
    box->connect_submit_signal(profile.make_event_slot<Scope>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_scope_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("ScopeFilterPanel", properties, [] (auto& profile) {
    auto filter_panel =
      new ScopeFilterPanel(*new ScopeBox(populate_scope_box_model()));
    apply_widget_properties(filter_panel, profile.get_properties());
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    filter_panel->connect_submit_signal(
      [=] (const auto& scope, auto mode) {
        submit_slot(QString("Mode:%1 %2").
          arg(to_string<ScopeBox>(mode)).arg(print_scope(scope)));
      });
    return filter_panel;
  });
  return profile;
}

UiProfile Spire::make_scope_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto type_property = define_enum<int>(
    {{"TICKER", 0}, {"VENUE", 1}, {"COUNTRY", 2}});
  properties.push_back(make_standard_enum_property("type", type_property));
  auto profile = UiProfile("ScopeListItem", properties, [] (auto& profile) {
    auto& type = get<int>("type", profile.get_properties());
    auto scope = [&] {
      if(type.get() == 0) {
        auto ticker = parse_ticker("MRU.TSX");
        auto scope = Scope("Metro Inc.");
        scope += ticker;
        return scope;
      } else if(type.get() == 1) {
        auto venue = VENUES.from(Venues::ASX);
        auto scope = Scope(venue.m_description);
        scope += venue.m_venue;
        return scope;
      } else {
        auto country = Countries::US;
        auto scope = Scope(COUNTRIES.from(country).m_name);
        scope += country;
        return scope;
      }
    }();
    auto item = new ScopeListItem(scope);
    apply_widget_properties(item, profile.get_properties());
    item->setMinimumSize(0, 0);
    item->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    return item;
  });
  return profile;
}
