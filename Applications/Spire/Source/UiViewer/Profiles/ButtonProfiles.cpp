#include "Spire/UiViewer/Profiles/ButtonProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/SwitchButton.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto setup_checkable_profile(UiProfile& profile, CheckBox* check_box) {
    auto& label = get<QString>("label", profile.get_properties());
    check_box->set_label(label.get());
    apply_widget_properties(check_box, profile.get_properties());
    label.connect_changed_signal([=] (const auto& value) {
      check_box->set_label(value);
    });
    auto& checked = get<bool>("checked", profile.get_properties());
    checked.connect_changed_signal([=] (auto value) {
      if(check_box->get_current()->get() != value) {
        check_box->get_current()->set(value);
      }
    });
    check_box->get_current()->connect_update_signal([&] (auto is_checked) {
      checked.set(is_checked);
    });
    check_box->get_current()->connect_update_signal(
      profile.make_event_slot<bool>("CheckedSignal"));
    auto& read_only = get<bool>("read-only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      check_box->set_read_only(is_read_only);
    });
    auto& layout_direction = get<bool>("left-to-right",
      profile.get_properties());
    layout_direction.connect_changed_signal([=] (auto value) {
      if(value) {
        check_box->setLayoutDirection(Qt::LeftToRight);
      } else {
        check_box->setLayoutDirection(Qt::RightToLeft);
      }
    });
    return check_box;
  }

  void populate_check_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    properties.push_back(make_standard_property<bool>("checked"));
    properties.push_back(make_standard_property("label", QString("Click me!")));
    properties.push_back(make_standard_property<bool>("read-only"));
    properties.push_back(make_standard_property("left-to-right", true));
  }
}

UiProfile Spire::make_check_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile("CheckBox", properties, [] (auto& profile) {
    return setup_checkable_profile(profile, new CheckBox());
  });
}

UiProfile Spire::make_delete_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DeleteIconButton", properties, [] (auto& profile) {
    auto button = make_delete_icon_button();
    apply_widget_properties(button, profile.get_properties());
    button->connect_click_signal(profile.make_event_slot("ClickSignal"));
    return button;
  });
  return profile;
}

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  properties.push_back(make_standard_property<bool>("disable_on_click", false));
  auto profile = UiProfile("IconButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto button = make_icon_button(
      image_from_svg(":/Icons/demo.svg", scale(26, 26)), tooltip.get());
    apply_widget_properties(button, profile.get_properties());
    auto& disable_on_click =
      get<bool>("disable_on_click", profile.get_properties());
    auto click_signal_slot = profile.make_event_slot("ClickSignal");
    button->connect_click_signal([=, &disable_on_click] () {
      button->setDisabled(disable_on_click.get());
      click_signal_slot();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_icon_toggle_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  auto profile = UiProfile("IconToggleButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto button = make_icon_toggle_button(
      image_from_svg(":/Icons/demo.svg", scale(26, 26)), tooltip.get());
    apply_widget_properties(button, profile.get_properties());
    return button;
  });
  return profile;
}

UiProfile Spire::make_label_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString("Click me!")));
  auto default_style = R"(
    :press > :body {
      background_color: 0x7E71B8;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("LabelButton", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto button = make_label_button(label.get());
    apply_widget_properties(button, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*button, styles);
    });
    button->connect_click_signal(profile.make_event_slot("ClickSignal"));
    return button;
  });
  return profile;
}

UiProfile Spire::make_radio_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile("RadioButton", properties, [=] (auto& profile) {
    return setup_checkable_profile(profile, make_radio_button());
  });
}

UiProfile Spire::make_switch_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("checked"));
  auto profile = UiProfile("SwitchButton", properties, [] (auto& profile) {
    auto button = new SwitchButton();
    apply_widget_properties(button, profile.get_properties());
    link(button->get_current(), get<bool>("checked", profile.get_properties()));
    button->get_current()->connect_update_signal(
      profile.make_event_slot<bool>("CurrentSignal"));
    button->connect_submit_signal(
      profile.make_event_slot<bool>("SubmitSignal"));
    return button;
  });
  return profile;
}
