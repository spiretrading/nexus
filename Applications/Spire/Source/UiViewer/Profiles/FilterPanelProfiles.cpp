#include "Spire/UiViewer/Profiles/FilterPanelProfiles.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/ProfilePanels.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_closed_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("item_count", 7));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  properties.push_back(make_standard_property("checked_item", -1));
  properties.push_back(make_standard_property("unchecked_item", -1));
  properties.push_back(make_standard_property("insert_item", -1));
  properties.push_back(make_standard_property("remove_item", -1));
  auto profile = UiProfile("ClosedFilterPanel", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto model = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i < item_count.get(); ++i) {
      model->push({item_text.get() + QString("%1").arg(i), false});
    }
    auto& checked_item = get<int>("checked_item", profile.get_properties());
    checked_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->set(value, 1, true);
    });
    auto& unchecked_item = get<int>("unchecked_item", profile.get_properties());
    unchecked_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->set(value, 1, false);
    });
    auto& insert_item = get<int>("insert_item", profile.get_properties());
    insert_item.connect_changed_signal(
      [=, index = 0] (const auto& value) mutable {
        if(value < 0 || value > model->get_row_size()) {
          return;
        }
        model->insert({QString("newItem%1").arg(index++), false}, value);
      });
    auto& remove_item = get<int>("remove_item", profile.get_properties());
    remove_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->remove(value);
    });
    auto panel = new ClosedFilterPanel(model);
    apply_widget_properties(panel, profile.get_properties());
    auto submit_filter_slot = profile.make_event_slot<QString>("SubmitSignal");
    panel->connect_submit_signal(
      [=] (const std::shared_ptr<AnyListModel>& submission) {
        auto result = QString();
        for(auto i = 0; i < submission->get_size(); ++i) {
          result += to_text(submission->get(i)) + " ";
        }
        submit_filter_slot(result);
      });
    return panel;
  });
  return profile;
}

UiProfile Spire::make_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      background_color: 0xFFFFFF;
      border_color: transparent;
    }
  )";
  properties.push_back(
    make_style_property("style_sheet", std::move(default_style)));
  auto profile = UiProfile("FilterPanel", properties, [] (auto& profile) {
    auto filter_panel = make_filter_panel();
    apply_widget_properties(filter_panel, profile.get_properties());
    filter_panel->connect_reset_signal(profile.make_event_slot("ResetSignal"));
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& style) {
      update_widget_style(*filter_panel, style);
    });
    return filter_panel;
  });
  return profile;
}

UiProfile Spire::make_open_filter_panel_profile() {
  return setup_open_filter_panel_profile("OpenFilterPanel",
    [] {
      return new OpenFilterPanel(
        *new TagComboBox(populate_tag_combo_box_model()));
    });
}
