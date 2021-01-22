#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace Nexus;
using namespace Spire;

UiProfile Spire::make_checkbox_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_bool_property("checked"));
  properties.push_back(make_standard_qstring_property("label",
    QString::fromUtf8("Click me!")));
  properties.push_back(make_standard_bool_property("read-only"));
  properties.push_back(make_standard_bool_property("left-to-right", true));
  auto profile = UiProfile(QString::fromUtf8("Checkbox"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto checkbox = new Checkbox(label.get());
      apply_widget_properties(checkbox, profile.get_properties());
      auto& checked = get<bool>("checked", profile.get_properties());
      checked.connect_changed_signal([=] (auto value) {
        if(value) {
          checkbox->setCheckState(Qt::Checked);
        } else {
          checkbox->setCheckState(Qt::Unchecked);
        }
      });
      QObject::connect(checkbox, &Checkbox::stateChanged, [&] (auto value) {
        checked.set(value == Qt::Checked);
      });
      QObject::connect(checkbox, &Checkbox::stateChanged,
        profile.make_event_slot<int>(QString::fromUtf8("stateChanged")));
      auto& read_only = get<bool>("read-only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        checkbox->set_read_only(is_read_only);
      });
      auto& layout_direction = get<bool>("left-to-right",
        profile.get_properties());
      layout_direction.connect_changed_signal([=] (auto value) {
        if(value) {
          checkbox->setLayoutDirection(Qt::LeftToRight);
        } else {
          checkbox->setLayoutDirection(Qt::RightToLeft);
        }
      });
      return checkbox;
    });
  return profile;
}

UiProfile Spire::make_color_selector_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qcolor_property("color"));
  auto profile = UiProfile(QString::fromUtf8("ColorSelectorButton"), properties,
    [] (auto& profile) {
      auto& color = get<QColor>("color", profile.get_properties());
      auto button = new ColorSelectorButton(color.get());
      apply_widget_properties(button, profile.get_properties());
      color.connect_changed_signal([=] (const auto& value) {
        button->set_color(value);
      });
      button->connect_color_signal([&] (auto value) {
        color.set(value);
      });
      button->connect_color_signal(
        profile.make_event_slot<QColor>(QString::fromUtf8("ColorSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_currency_combo_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_currency_property("currency"));
  auto profile = UiProfile(QString::fromUtf8("CurrencyComboBox"), properties,
    [] (auto& profile) {
      auto combo_box = new CurrencyComboBox(GetDefaultCurrencyDatabase());
      apply_widget_properties(combo_box, profile.get_properties());
      auto& currency = get<CurrencyId>("currency", profile.get_properties());
      currency.set(combo_box->get_currency());
      currency.connect_changed_signal([=] (auto value) {
        combo_box->set_currency(value);
      });
      combo_box->connect_selected_signal([&] (auto value) {
        currency.set(value);
      });
      combo_box->connect_selected_signal(profile.make_event_slot<CurrencyId>(
        QString::fromUtf8("SelectedSignal")));
      return combo_box;
    });
  return profile;
}

UiProfile Spire::make_flat_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("label",
    QString::fromUtf8("Click me!")));
  auto profile = UiProfile(QString::fromUtf8("FlatButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_flat_button(label.get());
      apply_widget_properties(button, profile.get_properties());
      label.connect_changed_signal([=] (const auto& value) {
        button->set_label(value);
      });
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("tooltip",
    QString::fromUtf8("Tooltip")));
  properties.push_back(make_standard_bool_property("checkable"));
  properties.push_back(make_standard_bool_property("checked"));
  auto profile = UiProfile(QString::fromUtf8("IconButton"), properties,
    [] (auto& profile) {
      auto button = new IconButton(imageFromSvg(":/Icons/demo.svg",
        scale(26, 26)));
      apply_widget_properties(button, profile.get_properties());
      auto& tooltip = get<QString>("tooltip", profile.get_properties());
      button->setToolTip(tooltip.get());
      tooltip.connect_changed_signal([=] (const auto& value) {
        button->setToolTip(value);
      });
      auto& checkable = get<bool>("checkable", profile.get_properties());
      checkable.connect_changed_signal([=] (auto is_checkable) {
        button->setCheckable(is_checkable);
        button->update();
      });
      auto& checked = get<bool>("checked", profile.get_properties());
      checked.connect_changed_signal([=] (auto is_checked) {
        button->setChecked(is_checked);
      });
      QObject::connect(button, &IconButton::clicked, [&] (auto is_checked) {
        checked.set(is_checked);
      });
      QObject::connect(button, &IconButton::clicked,
        profile.make_event_slot<bool>(QString::fromUtf8("clicked")));
      return button;
    });
  return profile;
}
