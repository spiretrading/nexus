#include "Spire/UiViewer/Profiles/MenuProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_check_button_menu_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("checked"));
  properties.push_back(make_standard_property("label", QString("Click me")));
  return UiProfile("CheckButtonMenuItem", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto item = new CheckButtonMenuItem(label.get());
    apply_widget_properties(item, profile.get_properties());
    auto& checked = get<bool>("checked", profile.get_properties());
    checked.connect_changed_signal([=] (auto value) {
      if(item->get_current()->get() != value) {
        item->get_current()->set(value);
      }
    });
    item->get_current()->connect_update_signal([&checked] (auto is_checked) {
      checked.set(is_checked);
    });
    item->get_current()->connect_update_signal(
      profile.make_event_slot<bool>("CheckedSignal"));
    return item;
  });
}

UiProfile Spire::make_context_menu_profile() {
  auto profile = UiProfile(QString::fromUtf8("ContextMenu"), {},
    [] (auto& profile) {
      auto button = make_label_button(QString::fromUtf8("Click me"));
      button->connect_click_signal([=, &profile] {
        auto menu = new ContextMenu(*button);
        auto file_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        file_menu->add_action("New",
          profile.make_event_slot<>(QString("Action:New")));
        file_menu->add_action("Save",
          profile.make_event_slot<>(QString("Action:Save")));
        file_menu->add_action("Save As...",
          profile.make_event_slot<>(QString("Action:Save As")));
        menu->add_menu("File", *file_menu);
        auto edit_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        edit_menu->add_action("Cut",
          profile.make_event_slot<>(QString("Action:Cut")));
        edit_menu->add_action("Copy",
          profile.make_event_slot<>(QString("Action:Copy")));
        edit_menu->add_action("Paste",
          profile.make_event_slot<>(QString("Action:Paste")));
        edit_menu->add_separator();
        edit_menu->add_action("Find...",
          profile.make_event_slot<>(QString("Action:Find")));
        edit_menu->add_action("Find Next",
          profile.make_event_slot<>(QString("Action:Find Next")));
        edit_menu->add_action("Find Previous",
          profile.make_event_slot<>(QString("Action:Find Previous")));
        edit_menu->add_separator();
        edit_menu->add_action("Select All",
          profile.make_event_slot<>(QString("Action:Select All")));
        edit_menu->add_action("Select None",
          profile.make_event_slot<>(QString("Action:Select None")));
        edit_menu->add_action("Select Inverse",
          profile.make_event_slot<>(QString("Action:Select Inverse")));
        menu->add_menu("Edit", *edit_menu);
        auto view_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        view_menu->add_action("Large",
          profile.make_event_slot<>(QString("Action:Large")));
        view_menu->add_action("Medium",
          profile.make_event_slot<>(QString("Action:Medium")));
        view_menu->add_action("Small",
          profile.make_event_slot<>(QString("Action:Small")));
        view_menu->add_separator();
        auto empty_menu = new ContextMenu(*static_cast<QWidget*>(view_menu));
        view_menu->add_menu("Empty", *empty_menu);
        menu->add_menu("View", *view_menu);
        auto sort_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        sort_menu->add_action("Name",
          profile.make_event_slot<>(QString("Action:Name")));
        sort_menu->add_action("Size",
          profile.make_event_slot<>(QString("Action:Size")));
        auto type_menu = new ContextMenu(*static_cast<QWidget*>(sort_menu));
        type_menu->add_action("Ticker",
          profile.make_event_slot<>(QString("Action:Ticker")));
        type_menu->add_action("Side",
          profile.make_event_slot<>(QString("Action:Side")));
        sort_menu->add_menu("Type", *type_menu);
        menu->add_menu("Sort by", *sort_menu);
        auto overflow_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        for(auto i = 1; i <= 100; ++i) {
          auto name = QString("Item %1").arg(i);
          overflow_menu->add_action(name,
            profile.make_event_slot<>(QString("Action:%1").arg(name)));
        }
        menu->add_menu("Overflow", *overflow_menu);
        menu->add_separator();
        menu->add_action("Refresh",
          profile.make_event_slot<>(QString("Action:Refresh")));
        menu->add_action("Export...",
          profile.make_event_slot<>(QString("Action:Export")));
        menu->add_separator();
        auto date_model = std::make_shared<LocalBooleanModel>();
        date_model->set(true);
        date_model->connect_update_signal(
          profile.make_event_slot<bool>(QString("Date CheckedSignal")));
        menu->add_check_box("Date", date_model);
        auto time_model = menu->add_check_box("Time");
        time_model->connect_update_signal(
          profile.make_event_slot<bool>(QString("Time CheckedSignal")));
        menu->add_separator();
        menu->add_action("This is a long name for test",
          profile.make_event_slot<>(
            QString("Action:This is a long name for test")));
        show_under_cursor(*menu);
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_menu_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("item_count", 3));
  properties.push_back(make_standard_property<QString>("item_label", "Item"));
  properties.push_back(
    make_standard_property<QString>("empty_message", "Empty"));
  auto profile = UiProfile("MenuButton", properties, [] (auto& profile) {
    auto label = make_label("MenuButton");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).
        set(border(scale_width(1), QColor(0xC8C8C8))).
        set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
        set(horizontal_padding(scale_width(8))).
        set(vertical_padding(scale_height(5)));
    });
    auto menu_button = new MenuButton(*label);
    menu_button->setFixedWidth(scale_width(120));
    auto& item_label = get<QString>("item_label", profile.get_properties());
    auto& item_count = get<int>("item_count", profile.get_properties());
    item_count.connect_changed_signal([=, &item_label, &profile] (auto count) {
      menu_button->get_menu().reset();
      for(auto i = 0; i < count; ++i) {
        auto item_name = QString("%1%2").arg(item_label.get()).arg(i + 1);
        menu_button->get_menu().add_action(item_name,
          profile.make_event_slot<>(QString("Action:%1").arg(item_name)));
      }
    });
    item_label.connect_changed_signal(
      [=, &item_count, &profile] (const auto& value) {
      menu_button->get_menu().reset();
      for(auto i = 0; i < item_count.get(); ++i) {
        auto item_name = QString("%1%2").arg(value).arg(i + 1);
        menu_button->get_menu().add_action(item_name,
          profile.make_event_slot<>(QString("Action:%1").arg(item_name)));
      }
    });
    auto& empty_message =
      get<QString>("empty_message", profile.get_properties());
    empty_message.connect_changed_signal([=] (const auto& value) {
      menu_button->set_empty_message(value);
    });
    apply_widget_properties(menu_button, profile.get_properties());
    return menu_button;
  });
  return profile;
}

UiProfile Spire::make_menu_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  auto profile = UiProfile("MenuIconButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto menu_button = make_menu_icon_button(
      image_from_svg(":/Icons/toolbar_icons/blotter.svg", scale(26, 26)),
      tooltip.get());
    apply_widget_properties(menu_button, profile.get_properties());
    auto& menu = menu_button->get_menu();
    menu.add_action("New...", profile.make_event_slot<>(QString("Action:New")));
    menu.add_separator();
    menu.add_action("Global",
      profile.make_event_slot<>(QString("Action:Global")));
    menu.add_action("Australia",
      profile.make_event_slot<>(QString("Action:Australia")));
    menu.add_action("North America",
      profile.make_event_slot<>(QString("Action:North America")));
    return menu_button;
  });
  return profile;
}

UiProfile Spire::make_menu_label_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("MenuLabelButton", properties, [] (auto& profile) {
    auto menu_button = make_menu_label_button("Window Manager");
    auto& menu = menu_button->get_menu();
    menu.add_action("Minimize All",
      profile.make_event_slot<>(QString("Action:Minimize All")));
    menu.add_action("Restore All",
      profile.make_event_slot<>(QString("Action:Restore All")));
    menu.add_action("Import Settings...",
      profile.make_event_slot<>(QString("Action:Import Settings")));
    menu.add_action("Export Settings...",
      profile.make_event_slot<>(QString("Action:Export Settings")));
    menu_button->setFixedWidth(scale_width(130));
    apply_widget_properties(menu_button, profile.get_properties());
    return menu_button;
  });
  return profile;
}
