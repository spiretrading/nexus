#include "Spire/UiViewer/Profiles/KeyProfiles.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/KeyListBox.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Spire;

namespace {
  auto populate_key_sequence_list() {
    auto model = std::make_shared<ArrayListModel<QKeySequence>>();
    model->push(QKeySequence(Qt::Key_F1));
    model->push(QKeySequence(Qt::SHIFT | Qt::Key_F1));
    model->push(QKeySequence(Qt::CTRL | Qt::Key_F2));
    return model;
  }
}

UiProfile Spire::make_key_filter_panel_profile() {
  return setup_open_filter_panel_profile("KeyFilterPanel",
    [] {
      return new KeyFilterPanel(
        *new KeyListBox(std::make_shared<ArrayListModel<QKeySequence>>(),
          populate_key_input_box_model(QKeySequence())));
    });
}

UiProfile Spire::make_key_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "F1"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("KeyInputBox", properties, [] (auto& profile) {
    auto& current = get<QString>("current", profile.get_properties());
    auto box = new KeyInputBox(populate_key_input_box_model(current.get()));
    box->setFixedWidth(scale_width(100));
    apply_widget_properties(box, profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value.isEmpty()) {
        if(box->get_current()->get() != QKeySequence()) {
          box->get_current()->set(QKeySequence());
        }
      } else {
        auto sequence = QKeySequence(value);
        if(sequence.count() != 0 && sequence[0] != Qt::Key::Key_unknown &&
            box->get_current()->get() != sequence) {
          box->get_current()->set(sequence);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&KeyInputBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QKeySequence>("Current"));
    box->get_current()->connect_update_signal([&current] (const auto& value) {
      current.set(value.toString());
    });
    box->connect_submit_signal(profile.make_event_slot<QKeySequence>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_key_list_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("KeyListBox", properties, [] (auto& profile) {
    auto& current = get<QString>("current", profile.get_properties());
    auto box = new KeyListBox(populate_key_sequence_list(),
      populate_key_input_box_model(QKeySequence(current.get())));
    box->setMinimumWidth(scale_width(200));
    apply_widget_properties(box, profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value.isEmpty()) {
        if(box->get_current()->get() != QKeySequence()) {
          box->get_current()->set(QKeySequence());
        }
      } else {
        auto sequence = QKeySequence(value);
        if(sequence.count() != 0 && sequence[0] != Qt::Key::Key_unknown &&
            box->get_current()->get() != sequence) {
          box->get_current()->set(sequence);
        }
      }
    });
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QKeySequence>("Current"));
    box->get_current()->connect_update_signal([&current] (const auto& value) {
      current.set(value.toString());
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&KeyListBox::set_read_only, box));
    auto keys_slot = profile.make_event_slot<QString>("Keys");
    auto print_keys = [=] {
      auto result = QString();
      auto keys = box->get_keys();
      for(auto i = 0; i < keys->get_size(); ++i) {
        result += QString("[%1] ").arg(to_text(keys->get(i)));
      }
      keys_slot(result);
    };
    box->get_keys()->connect_operation_signal([=] (const auto& operation) {
      visit(operation,
        [&] (const KeySequenceListModel::AddOperation&) {
          print_keys();
        },
        [&] (const KeySequenceListModel::RemoveOperation&) {
          print_keys();
        });
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_key_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("key", "f1"));
  auto profile = UiProfile("KeyTag", properties, [] (auto& profile) {
    auto key_tag = new KeyTag();
    apply_widget_properties(key_tag, profile.get_properties());
    auto& key = get<QString>("key", profile.get_properties());
    key.connect_changed_signal([=] (auto key_text) {
      auto key = [&] {
        if(key_text.toLower() == "alt") {
          return Qt::Key_Alt;
        } else if(key_text.toLower() == "ctrl") {
          return Qt::Key_Control;
        } else if(key_text.toLower() == "shift") {
          return Qt::Key_Shift;
        } else if(auto sequence = QKeySequence::fromString(key_text);
            !sequence.isEmpty()) {
          return Qt::Key(sequence[0]);
        }
        return Qt::Key_unknown;
      }();
      key_tag->get_current()->set(key);
    });
    return key_tag;
  });
  return profile;
}
