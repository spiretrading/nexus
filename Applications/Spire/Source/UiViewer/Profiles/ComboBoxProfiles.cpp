#include "Spire/UiViewer/Profiles/ComboBoxProfiles.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

UiProfile Spire::make_combo_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "Car"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("ComboBox", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalQueryModel<QString>>();
    model->add("Almond");
    model->add("Amber");
    model->add("Amberose");
    model->add("Apple");
    model->add("Beige");
    model->add("Bronze");
    model->add("Brown");
    model->add("Black");
    model->add("Car");
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model = make_local_value_model(current.get());
    auto box =
      new ComboBox(model, current_model, &ListView::default_item_builder);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<std::any>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      if(auto value = model->parse(current)) {
        box->get_current()->set(*value);
      } else {
        auto current_blocker = shared_connection_block(current_connection);
        box->get_current()->set(current);
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ComboBox<QString>::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_drop_down_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("item_count", 15));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  auto profile = UiProfile("DropDownBox", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < item_count.get(); ++i) {
      list_model->push(item_text.get() + QString("%1").arg(i));
    }
    auto drop_down_box = new DropDownBox(list_model);
    drop_down_box->setFixedWidth(scale_width(112));
    apply_widget_properties(drop_down_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      drop_down_box->set_read_only(is_read_only);
    });
    auto current_slot = profile.make_event_slot<optional<std::any>>("Current");
    drop_down_box->get_current()->set(list_model->get_size() - 1);
    drop_down_box->get_current()->connect_update_signal(
      [=] (auto current) {
        if(current) {
          current_slot(drop_down_box->get_list()->get(*current));
        } else {
          current_slot(none);
        }
      });
    drop_down_box->connect_submit_signal(
      profile.make_event_slot<optional<std::any>>("Submit"));
    return drop_down_box;
  });
  return profile;
}

UiProfile Spire::make_drop_down_list_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("item_count", 15));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  auto profile = UiProfile("DropDownList", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto button = make_label_button("DropDownList");
    button->connect_click_signal([&, button] {
      auto list_model = std::make_shared<ArrayListModel<QString>>();
      for(auto i = 0; i < item_count.get(); ++i) {
        list_model->push(item_text.get() + QString("%1").arg(i));
      }
      auto list_view =
        new ListView(list_model,
          [&] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
            return make_label(model->get(index));
          });
      auto drop_down_list = new DropDownList(*list_view, *button);
      drop_down_list->window()->setAttribute(Qt::WA_DeleteOnClose);
      drop_down_list->show();
    });
    return button;
  });
  return profile;
}
