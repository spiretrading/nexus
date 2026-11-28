#include "Spire/UiViewer/Profiles/VenueProfiles.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/VenueBox.hpp"
#include "Spire/Ui/VenueFilterPanel.hpp"
#include "Spire/Ui/VenueListBox.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Nexus;
using namespace Spire;

UiProfile Spire::make_venue_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "ASX"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("VenueBox", properties, [] (auto& profile) {
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model =
      std::make_shared<LocalVenueModel>(Venue(current.get().toStdString()));
    auto box = make_venue_box(current_model);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    current.connect_changed_signal([=] (const auto& current) {
      if(auto venue =
          VENUES.from_display_name(current.toUpper().toStdString()).m_venue) {
        box->get_current()->set(venue);
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&VenueBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<Venue>("Current"));
    box->connect_submit_signal(profile.make_event_slot<Venue>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_venue_filter_panel_profile() {
  return setup_open_filter_panel_profile("VenueFilterPanel",
    [] { return make_venue_filter_panel(); });
}

UiProfile Spire::make_venue_list_box_profile() {
  return setup_tag_combo_box_profile("VenueListBox",
    [] { return make_venue_list_box(); });
}
