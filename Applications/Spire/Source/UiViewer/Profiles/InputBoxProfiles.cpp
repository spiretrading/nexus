#include "Spire/UiViewer/Profiles/InputBoxProfiles.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/ScopeBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_editable_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto test_widget_property = define_enum<int>(
    {{"TextBox", 0}, {"DropDownBox", 1}, {"DecimalBox", 2}, {"QuantityBox", 3},
      {"KeyInputBox", 4}, {"ScopeBox", 5}});
  properties.push_back(
    make_standard_enum_property("input_box", test_widget_property));
  auto profile = UiProfile("EditableBox", properties, [] (auto& profile) {
    auto& test_widget = get<int>("input_box", profile.get_properties());
    auto input_box = [&] {
      auto value = test_widget.get();
      if(value == 0) {
        return new EditableBox(*new TextBox("TextBox"));
      } else if(value == 1) {
        auto list_model = std::make_shared<ArrayListModel<QString>>();
        for(auto i = 0; i < 5; ++i) {
          list_model->push(QString("item%1").arg(i));
        }
        return new EditableBox(*new DropDownBox(list_model));
      } else if(value == 2) {
        return new EditableBox(*new DecimalBox(
          std::make_shared<LocalOptionalDecimalModel>(Decimal(1))));
      } else if(value == 3) {
        return new EditableBox(*new QuantityBox(
          std::make_shared<LocalOptionalQuantityModel>(Quantity(1))));
      } else if(value == 4) {
        return new EditableBox(*new KeyInputBox(
          populate_key_input_box_model(QKeySequence("F1"))),
          [] (const auto& sequence) {
            return key_input_box_validator(sequence) == QValidator::Acceptable;
          });
      }
      auto query_model = populate_scope_box_model();
      auto current = std::make_shared<LocalValueModel<Scope>>();
      current->set(std::any_cast<Scope>(*query_model->parse("TSX")));
      return new EditableBox(*new ScopeBox(query_model, current));
    }();
    input_box->setMinimumWidth(scale_width(112));
    apply_widget_properties(input_box, profile.get_properties());
    return input_box;
  });
  return profile;
}

UiProfile Spire::make_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property<QString>("label", "Label"));
  auto profile = UiProfile("InputBox", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto input_box = make_input_box(make_label(label.get()));
    apply_widget_properties(input_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      if(is_read_only) {
        match(*input_box, ReadOnly());
      } else {
        unmatch(*input_box, ReadOnly());
      }
    });
    return input_box;
  });
  return profile;
}

UiProfile Spire::make_search_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  auto profile = UiProfile("SearchBox", properties, [] (auto& profile) {
    auto search_box = new SearchBox();
    search_box->setFixedWidth(scale_width(180));
    apply_widget_properties(search_box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      search_box->set_placeholder(text);
    });
    search_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    search_box->connect_submit_signal(
      profile.make_event_slot<QString>("Submit"));
    return search_box;
  });
  return profile;
}

UiProfile Spire::make_text_area_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property<int>("line-height", 120));
  auto horizontal_alignment_property = define_enum<Qt::Alignment>(
    {{"LEFT", Qt::AlignLeft},
     {"RIGHT", Qt::AlignRight},
     {"CENTER", Qt::AlignHCenter},
     {"JUSTIFY", Qt::AlignJustify}});
  properties.push_back(make_standard_enum_property(
    "horizontal-align", horizontal_alignment_property));
  auto profile = UiProfile("TextAreaBox", properties, [] (auto& profile) {
    auto text_area_box = new TextAreaBox();
    apply_widget_properties(text_area_box, profile.get_properties());
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& value) {
      if(text_area_box->get_current()->get() != value) {
        text_area_box->get_current()->set(value);
      }
    });
    text_area_box->get_current()->connect_update_signal(
      [&] (const auto& value) {
        if(current.get() != value) {
          current.set(value);
        }
      });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      text_area_box->set_read_only(is_read_only);
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      text_area_box->set_placeholder(text);
    });
    auto& line_height = get<int>("line-height", profile.get_properties());
    line_height.connect_changed_signal(
      [=] (auto line_height) {
        update_style(*text_area_box, [&] (auto& style) {
          style.get(Any()).set(LineHeight(
            static_cast<double>(line_height) / 100));
        });
      });
    auto& horizontal_alignment = get<Qt::Alignment>("horizontal-align",
      profile.get_properties());
    horizontal_alignment.connect_changed_signal(
      [&, text_area_box] (auto alignment) {
        update_style(*text_area_box, [&] (auto& style) {
          style.get(Any()).
            set(TextAlign(
              Qt::Alignment(alignment) | Qt::AlignTop));
        });
      });
    text_area_box->connect_submit_signal(
      profile.make_event_slot<QString>("Submit"));
    text_area_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    return text_area_box;
  });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property<QString>("rejected", "deny"));
  auto default_style = R"(
    any {
      horizontal_padding: 8px;
      border_size: 1px;
      text_align: left;
    }
    :rejected {
      background_color: chain(timeout(0xFFF1F1, 250ms),
        linear(0xFFF1F1, revert, 300ms));
      border_color: chain(timeout(0xB71C1C, 550ms), revert);
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("TextBox", properties, [] (auto& profile) {
    auto& rejected = get<QString>("rejected", profile.get_properties());
    auto model = make_validated_value_model<QString>(
      [&] (const QString& value) {
        if(value == rejected.get()) {
          return QValidator::State::Intermediate;
        }
        return QValidator::State::Acceptable;
      });
    auto text_box = new TextBox(model);
    text_box->setFixedWidth(scale_width(100));
    apply_widget_properties(text_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      text_box->set_read_only(is_read_only);
    });
    link(text_box->get_current(),
      get<QString>("current", profile.get_properties()));
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      text_box->set_placeholder(text);
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*text_box, styles);
    });
    text_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    text_box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    text_box->connect_reject_signal(profile.make_event_slot<QString>("Reject"));
    return text_box;
  });
  return profile;
}
