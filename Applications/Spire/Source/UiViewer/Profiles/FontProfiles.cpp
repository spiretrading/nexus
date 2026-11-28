#include "Spire/UiViewer/Profiles/FontProfiles.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FontBox.hpp"
#include "Spire/Ui/FontFamilyBox.hpp"
#include "Spire/Ui/FontStyleBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_font_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_size_properties("parent_width", "parent_height", properties);
  populate_font_properties(properties, "current");
  auto profile = UiProfile("FontBox", properties, [] (auto& profile) {
    auto font_box = new FontBox();
    auto body_widget = new QWidget();
    auto layout = make_vbox_layout(body_widget);
    layout->addWidget(font_box);
    layout->addSpacing(scale_height(20));
    layout->addStretch(1);
    auto label = make_label("Handgloves");
    layout->addWidget(label);
    auto box = new Box(body_widget);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0x4B23A0)));
    });
    apply_widget_size_properties(box, "parent_width", "parent_height",
      profile.get_properties());
    box->setFixedWidth(scale_width(200));
    auto& current = get<QFont>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto& font) {
      if(font_box->get_current()->get() != font) {
        font_box->get_current()->set(font);
      }
    });
    auto current_slot =
      profile.make_event_slot<QString>(QString::fromUtf8("Current"));
    font_box->get_current()->connect_update_signal([=] (auto& font) {
      update_style(*label, [&] (auto& style) {
        style.get(Any()).set(Font(font));
      });
      current_slot(QString("%1, %2, %3").arg(font.family()).
        arg(QFontDatabase().styleString(font)).
        arg(unscale_width(font.pixelSize())));
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_font_family_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<QString>(
    {{"Roboto", "Roboto"}, {"Source Sans Pro", "Source Sans Pro"},
    {"Tahoma", "Tahoma"}, {"Times New Roman", "Times New Roman"}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("FontFamilyBox", properties, [] (auto& profile) {
    auto box = make_font_family_box("Roboto");
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&FontFamilyBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_font_style_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto font_family_property = define_enum<QString>(
    {{"Roboto", "Roboto"}, {"Source Sans Pro", "Source Sans Pro"},
    {"Tahoma", "Tahoma"}, {"Times New Roman", "Times New Roman"},
    {"System", "System"}, {"Cambria Math", "Cambria Math"},
    {"Webdings", "Webdings"}});
  populate_enum_properties(properties, "font_family", font_family_property);
  properties.push_back(make_standard_property<QString>("current", "Regular"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("FontStyleBox", properties, [] (auto& profile) {
    auto family_model = make_local_value_model<QString>("Roboto");
    auto box = make_font_style_box(family_model);
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& font_family = get<QString>("font_family", profile.get_properties());
    font_family.connect_changed_signal([=] (auto& value) {
      family_model->set(value);
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto& value) {
      auto styles = QFontDatabase().styles(family_model->get());
      if(styles.contains(value, Qt::CaseInsensitive)) {
        if(box->get_current()->get() != value) {
          box->get_current()->set(value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&FontStyleBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<std::any>("Current"));
    box->get_current()->connect_update_signal([&current] (const auto& value) {
      current.set(value);
    });
    box->connect_submit_signal(profile.make_event_slot<std::any>("Submit"));
    return box;
  });
  return profile;
}
