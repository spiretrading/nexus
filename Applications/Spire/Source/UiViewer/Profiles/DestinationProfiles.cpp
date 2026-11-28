#include "Spire/UiViewer/Profiles/DestinationProfiles.hpp"
#include "Nexus/Definitions/StandardDestinations.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/DestinationFilterPanel.hpp"
#include "Spire/Ui/DestinationListBox.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Nexus;
using namespace Spire;

UiProfile Spire::make_destination_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "TSX"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("DestinationBox", properties, [] (auto& profile) {
    auto selection = DESTINATIONS.select_all([] (auto& value) { return true; });
    auto destinations =
      std::make_shared<ArrayListModel<DestinationDatabase::Entry>>();
    for(auto& destination : selection) {
      destinations->push(destination);
    }
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model =
      std::make_shared<LocalDestinationModel>(current.get().toStdString());
    auto box = make_destination_box(current_model, destinations);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<Destination>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      box->get_current()->set(current.toStdString());
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&DestinationBox::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<Destination>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_destination_filter_panel_profile() {
  return setup_open_filter_panel_profile("DestinationFilterPanel",
    [] { return make_destination_filter_panel(); });
}

UiProfile Spire::make_destination_list_box_profile() {
  return setup_tag_combo_box_profile("DestinationListBox",
    [] { return make_destination_list_box(); });
}

UiProfile Spire::make_destination_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DestinationListItem", properties,
    [] (auto& profile) {
      auto item = new DestinationListItem(DESTINATIONS.from(Destinations::TSX));
      apply_widget_properties(item, profile.get_properties());
      return item;
    });
  return profile;
}
