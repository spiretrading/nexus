#include "Spire/UiViewer/Profiles/DisplayProfiles.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Identicon.hpp"
#include "Spire/Ui/ProgressBar.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_identicon_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<int>("id", 12345));
  auto profile = UiProfile("Identicon", properties, [] (auto& profile) {
    auto& id = get<int>("id", profile.get_properties());
    auto size = scale(40, 40);
    auto icon = new Icon(make_identicon(
      DirectoryEntry::make_account(static_cast<unsigned int>(id.get())), size));
    icon->setFixedSize(size);
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(none));
    set_style(*icon, style);
    id.connect_changed_signal([=] (auto value) {
      update_style(*icon, [&] (auto& style) {
        style.get(Any()).
          set(IconImage(make_identicon(
            DirectoryEntry::make_account(static_cast<unsigned int>(value)),
            size)));
      });
    });
    apply_widget_properties(icon, profile.get_properties());
    return icon;
  });
  return profile;
}

UiProfile Spire::make_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("label", QString("Label")));
  auto profile = UiProfile("Label", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto label_box = make_label(label.get());
    return label_box;
  });
  return profile;
}

UiProfile Spire::make_number_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto number_type_property = define_enum<QString>(
    {{"Integer", "int"}, {"Unsigned Integer", "uint"},
    {"Integer64", "int64"}, {"Unsigned Integer64", "uint64"},
    {"double", "double"}, {"Quantity", "Quantity"}, {"Money", "Money"}});
  populate_enum_properties(properties, "number_type", number_type_property);
  properties.push_back(make_standard_property<Decimal>("number", 1000000));
  properties.push_back(make_standard_property("omit_group_separator", false));
  auto profile = UiProfile("NumberLabel", properties, [] (auto& profile) {
    auto convert =
      [&] (double value, const QString& type, bool is_omit_separator) {
        auto locale = QLocale();
        if(is_omit_separator) {
          locale.setNumberOptions(QLocale::OmitGroupSeparator);
        }
        if(type == "int") {
          return to_text(static_cast<int>(value), locale);
        } else if(type == "uint") {
          return to_text(static_cast<unsigned int>(value), locale);
        } else if(type == "int64") {
          return to_text(static_cast<std::int64_t>(value), locale);
        } else if(type == "uint64") {
          return to_text(static_cast<std::uint64_t>(value), locale);
        } else if(type == "Quantity") {
          return to_text(Quantity(value), locale);
        } else if(type == "Money") {
          return to_text(Money(Quantity(value)), locale);
        }
        return to_text(value, locale);
      };
    auto& type = get<QString>("number_type", profile.get_properties());
    auto& number = get<Decimal>("number", profile.get_properties());
    auto& separator =
      get<bool>("omit_group_separator", profile.get_properties());
    auto label = std::make_shared<LocalTextModel>();
    type.connect_changed_signal([=, &number, &separator] (const auto& type) {
      label->set(
        convert(number.get().convert_to<double>(), type, separator.get()));
    });
    number.connect_changed_signal([=, &type, &separator] (const auto& value) {
      label->set(
        convert(value.convert_to<double>(), type.get(), separator.get()));
    });
    separator.connect_changed_signal([=, &type, &number] (auto value) {
      label->set(convert(number.get().convert_to<double>(), type.get(), value));
    });
    return make_label(label);
  });
  return profile;
}

UiProfile Spire::make_responsive_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("label1", QString("Hello world")));
  properties.push_back(make_standard_property("label2", QString("Hello w.")));
  properties.push_back(make_standard_property("label3", QString("Hello")));
  properties.push_back(make_standard_property("label4", QString("HW")));
  properties.push_back(make_standard_property("font-size", 12));
  auto profile = UiProfile("ResponsiveLabel", properties, [] (auto& profile) {
    auto labels = std::make_shared<ArrayListModel<QString>>();
    auto& label1 = get<QString>("label1", profile.get_properties());
    labels->push(label1.get());
    auto& label2 = get<QString>("label2", profile.get_properties());
    labels->push(label2.get());
    auto& label3 = get<QString>("label3", profile.get_properties());
    labels->push(label3.get());
    auto& label4 = get<QString>("label4", profile.get_properties());
    labels->push(label4.get());
    auto label = new ResponsiveLabel(labels);
    apply_widget_properties(label, profile.get_properties());
    auto label_map =
      std::make_shared<std::unordered_map<int, int>>();
    auto connect_label_changed_signal =
      [=] (auto& property, auto id, auto label_list) {
        label_map->insert_or_assign(id, id);
        property.connect_changed_signal([=] (const auto& value) {
          if(value.isEmpty()) {
            if(auto index = label_map->find(id); index != label_map->end()) {
              labels->remove(label_map->at(index->first));
              for(auto& label_property : *label_map) {
                if(label_property.second > index->second) {
                  label_property.second -= 1;
                }
              }
            }
            label_map->erase(id);
            return;
          } else if(auto index = label_map->find(id);
              index == label_map->end()) {
            labels->push(value);
            label_map->insert_or_assign(id, labels->get_size() - 1);
            return;
          }
          labels->set(label_map->at(id), value);
        });
      };
    connect_label_changed_signal(label1, 0, labels);
    connect_label_changed_signal(label2, 1, labels);
    connect_label_changed_signal(label3, 2, labels);
    connect_label_changed_signal(label4, 3, labels);
    auto& font_size = get<int>("font-size", profile.get_properties());
    font_size.connect_changed_signal([=] (auto size) {
      update_style(*label, [&] (auto& style) {
        style.get(Any()).set(FontSize(scale_height(size)));
      });
    });
    return label;
  });
  return profile;
}

UiProfile Spire::make_text_area_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<int>("width", 100));
  properties.push_back(make_standard_property("current", QString("Label")));
  auto profile = UiProfile("TextAreaLabel", properties, [] (auto& profile) {
    auto text_area_label = make_text_area_label("");
    auto& width = get<int>("width", profile.get_properties());
    width.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_width(text_area_label->width()) != value) {
          text_area_label->setFixedWidth(scale_width(value));
        }
      }
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& value) {
      text_area_label->get_current()->set(value);
    });
    return text_area_label;
  });
  return profile;
}

UiProfile Spire::make_progress_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  get<int>("width", properties).set(scale_width(224));
  properties.push_back(make_standard_property("current", 0));
  auto profile = UiProfile("ProgressBar", properties, [] (auto& profile) {
    auto& current = get<int>("current", profile.get_properties());
    auto current_model = std::make_shared<LocalProgressModel>(current.get());
    auto progress_bar = new ProgressBar(current_model);
    apply_widget_properties(progress_bar, profile.get_properties());
    current.connect_changed_signal([=] (const auto& current) {
      current_model->set(std::clamp(current, 0, 100));
    });
    progress_bar->get_current()->connect_update_signal(
      profile.make_event_slot<int>("Current"));
    return progress_bar;
  });
  return profile;
}
