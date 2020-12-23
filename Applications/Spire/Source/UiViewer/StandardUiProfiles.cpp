#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace Spire;

UiProfile Spire::make_check_box_profile() {
  auto enabled_property = make_standard_bool_property("enabled", true);
  auto width_property = make_standard_int_property("width");
  auto height_property = make_standard_int_property("height");
  auto text_property = make_standard_qstring_property("text",
    QString::fromUtf8("CheckBox"));
  auto profile = UiProfile(QString::fromUtf8("CheckBox"),
    {enabled_property, width_property, height_property, text_property},
    [] (UiProfile& profile) {
      auto& enabled = get<bool>("enabled", profile.get_properties());
      auto& width = get<int>("width", profile.get_properties());
      auto& height = get<int>("height", profile.get_properties());
      auto& text = get<QString>("text", profile.get_properties());
      auto check_box = make_check_box(text.get());
      check_box->setEnabled(enabled.get());
      enabled.connect_changed_signal([=] (bool value) {
        check_box->setEnabled(value);
      });
      width.connect_changed_signal([=] (int value) {
        check_box->setFixedSize(value, check_box->height());
      });
      height.connect_changed_signal([=] (int value) {
        check_box->setFixedSize(check_box->width(), value);
      });
      QObject::connect(check_box, &CheckBox::stateChanged,
        profile.make_event_slot<int>(QString::fromUtf8("stateChanged")));
      return check_box;
    });
  return profile;
}

UiProfile Spire::make_color_selector_button_profile() {
  auto enabled_property = make_standard_bool_property("enabled", true);
  auto width_property = make_standard_int_property("width", scale_width(100));
  auto height_property = make_standard_int_property("height", scale_height(26));
  auto color_property = make_standard_qcolor_property("color");
  auto profile = UiProfile(QString::fromUtf8("ColorSelectorButton"),
    {enabled_property, width_property, height_property, color_property},
    [] (UiProfile& profile) {
      auto& enabled = get<bool>("enabled", profile.get_properties());
      auto& width = get<int>("width", profile.get_properties());
      auto& height = get<int>("height", profile.get_properties());
      auto& color = get<QColor>("color", profile.get_properties());
      auto button = new ColorSelectorButton(color.get());
      button->setEnabled(enabled.get());
      button->setFixedSize(width.get(), height.get());
      enabled.connect_changed_signal([=] (bool value) {
        button->setEnabled(value);
      });
      width.connect_changed_signal([=] (int value) {
        button->setFixedSize(value, button->height());
      });
      height.connect_changed_signal([=] (int value) {
        button->setFixedSize(button->width(), value);
      });
      color.connect_changed_signal([=] (const QColor& value) {
        button->set_color(value);
      });
      button->connect_color_signal(
        profile.make_event_slot<QColor>(QString::fromUtf8("ColorSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_flat_button_profile() {
  auto enabled_property = make_standard_bool_property("enabled", true);
  auto width_property = make_standard_int_property("width", scale_width(100));
  auto height_property = make_standard_int_property("height", scale_height(26));
  auto label_property = make_standard_qstring_property("label",
    QString::fromUtf8("Click me!"));
  auto profile = UiProfile(QString::fromUtf8("FlatButton"),
    {enabled_property, width_property, height_property, label_property},
    [] (UiProfile& profile) {
      auto& enabled = get<bool>("enabled", profile.get_properties());
      auto& width = get<int>("width", profile.get_properties());
      auto& height = get<int>("height", profile.get_properties());
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_flat_button(label.get());
      button->setEnabled(enabled.get());
      button->setFixedSize(width.get(), height.get());
      enabled.connect_changed_signal([=] (bool value) {
        button->setEnabled(value);
      });
      width.connect_changed_signal([=] (int value) {
        button->setFixedSize(value, button->height());
      });
      height.connect_changed_signal([=] (int value) {
        button->setFixedSize(button->width(), value);
      });
      label.connect_changed_signal([=] (const QString& value) {
        button->set_label(value);
      });
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}
