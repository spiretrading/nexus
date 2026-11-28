#include "Spire/UiViewer/Profiles/TagProfiles.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Spire;

namespace {
  auto populate_tag_box_model() {
    auto model = std::make_shared<ArrayListModel<QString>>();
    model->push("CAN");
    model->push("MSFT.NSDQ");
    model->push("XIU.TSX");
    return model;
  }
}

UiProfile Spire::make_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("label", "CAN"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("Tag", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto tag = new Tag(label.get());
    label.connect_changed_signal([=] (const auto& value) {
      if(tag->get_label()->get() != value) {
        tag->get_label()->set(value);
      }
    });
    tag->get_label()->connect_update_signal([&] (const auto& value) {
      if(label.get() != value) {
        label.set(value);
      }
    });
    apply_widget_properties(tag, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      tag->set_read_only(is_read_only);
    });
    tag->connect_delete_signal(profile.make_event_slot("DeleteSignal"));
    tag->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    tag->setMinimumSize(0, 0);
    return tag;
  });
  return profile;
}

UiProfile Spire::make_tag_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property<QString>("add_tag"));
  auto profile = UiProfile("TagBox", properties, [] (auto& profile) {
    auto tag_box = new TagBox(populate_tag_box_model(),
      std::make_shared<LocalTextModel>());
    apply_widget_properties(tag_box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      tag_box->set_placeholder(text);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      tag_box->set_read_only(is_read_only);
    });
    auto& add_tag = get<QString>("add_tag", profile.get_properties());
    add_tag.connect_changed_signal([=] (const auto& value) {
      if(!value.isEmpty()) {
        tag_box->get_tags()->push(value);
      }
    });
    tag_box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return tag_box;
  });
  return profile;
}

UiProfile Spire::make_tag_combo_box_profile() {
  return setup_tag_combo_box_profile("TagComboBox",
    [] { return new TagComboBox(populate_tag_combo_box_model()); });
}
