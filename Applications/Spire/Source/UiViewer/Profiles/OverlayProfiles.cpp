#include "Spire/UiViewer/Profiles/OverlayProfiles.hpp"
#include <QPointer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DateFilterPanel.hpp"
#include "Spire/Ui/InfoPanel.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/UiViewer/Profiles/ProfilePanels.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

#ifdef ERROR
  #undef ERROR
#endif

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void create_child_panel(bool close_on_focus_out, bool draggable,
      OverlayPanel::Positioning positioning, Button* parent) {
    auto body = create_panel_body("Show child panel");
    auto panel = new OverlayPanel(*body, *parent);
    auto button = body->findChild<Button*>();
    button->connect_click_signal([=] {
      create_child_panel(close_on_focus_out, draggable, positioning, button);
    });
    panel->setAttribute(Qt::WA_DeleteOnClose);
    panel->set_closed_on_focus_out(close_on_focus_out);
    panel->set_is_draggable(draggable);
    panel->set_positioning(positioning);
    panel->show();
  }
}

UiProfile Spire::make_info_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("message", "Message"));
  auto severity_property = define_enum<InfoPanel::Severity>(
    {{"INFO", InfoPanel::Severity::INFO},
     {"SUCCESS", InfoPanel::Severity::SUCCESS},
     {"WARNING", InfoPanel::Severity::WARNING},
     {"ERROR", InfoPanel::Severity::ERROR}});
  properties.push_back(
    make_standard_enum_property("severity", severity_property));
  auto profile = UiProfile("InfoPanel", properties, [] (auto& profile) {
    auto button = make_label_button("Click me");
    auto& message = get<QString>("message", profile.get_properties());
    auto& severity =
      get<InfoPanel::Severity>("severity", profile.get_properties());
    auto info_panel = QPointer<InfoPanel>();
    button->connect_click_signal([=, &message, &severity] () mutable {
      if(info_panel) {
        return;
      }
      info_panel = new InfoPanel(severity.get(), message.get(), *button);
      info_panel->show();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_info_tip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString("Body Label")));
  properties.push_back(make_standard_property<bool>("interactive"));
  auto default_style = R"(
    any {
      padding_top: 8px;
      padding_right: 8px;
      padding_bottom: 8px;
      padding_left: 8px;
      border_size: 1px;
    }
    :read_only and :disabled {
      background_color: 0xFFFFFF;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("InfoTip", properties, [] (auto& profile) {
    auto button = make_label_button("Hover me!");
    auto body_label = make_label("");
    update_style(*body_label, [&] (auto& style) {
      style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    });
    auto info_tip = new InfoTip(body_label, button);
    apply_widget_properties(button, profile.get_properties());
    auto& label = get<QString>("label", profile.get_properties());
    label.connect_changed_signal([=] (auto value) {
      body_label->get_current()->set(value);
    });
    auto& interactive = get<bool>("interactive", profile.get_properties());
    interactive.connect_changed_signal([=] (bool is_interactive) {
      info_tip->set_interactive(is_interactive);
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*info_tip, styles);
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_overlay_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("close-on-focus-out", false));
  properties.push_back(make_standard_property("draggable", true));
  auto positioning_property = define_enum<OverlayPanel::Positioning>(
    {{"NONE", OverlayPanel::Positioning::NONE},
     {"PARENT", OverlayPanel::Positioning::PARENT}});
  properties.push_back(
    make_standard_enum_property("positioning", positioning_property));
  auto profile = UiProfile("OverlayPanel", properties, [=] (auto& profile) {
    auto& close_on_focus_out =
      get<bool>("close-on-focus-out", profile.get_properties());
    auto& draggable = get<bool>("draggable", profile.get_properties());
    auto& positioning =
      get<OverlayPanel::Positioning>("positioning", profile.get_properties());
    auto button = make_label_button("Click me");
    apply_widget_properties(button, profile.get_properties());
    auto panel = QPointer<OverlayPanel>();
    button->connect_click_signal(
      [=, &profile, &close_on_focus_out, &draggable, &positioning]
          () mutable {
        if(panel && !close_on_focus_out.get()) {
          return;
        }
        auto body = create_panel_body("Show child panel");
        panel = new OverlayPanel(*body, *button);
        auto child_button = body->findChild<Button*>();
        child_button->connect_click_signal(
          [=, &close_on_focus_out, &draggable, &positioning] {
            create_child_panel(close_on_focus_out.get(), draggable.get(),
              positioning.get(), child_button);
          });
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->set_closed_on_focus_out(close_on_focus_out.get());
        panel->set_is_draggable(draggable.get());
        panel->set_positioning(positioning.get());
        panel->show();
      });
    return button;
  });
  return profile;
}

UiProfile Spire::make_popup_box_profile() {
    auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto panel_property = define_enum<int>(
    {{"Custom Panel", 0}, {"FilterPanel", 1}, {"DateFilterPanel", 2}});
  properties.push_back(
    make_standard_enum_property("panel", panel_property));
  auto directions_property = define_enum<Qt::Orientations>(
    {{"BOTH", Qt::Horizontal | Qt::Vertical}, {"HORIZONTAL", Qt::Horizontal},
     {"VERTICAL", Qt::Vertical}, {"NONE", {}}});
  properties.push_back(
    make_standard_enum_property("overflow_directions", directions_property));
  auto profile = UiProfile("PopupBox", properties, [] (auto& profile) {
    auto& panel = get<int>("panel", profile.get_properties());
    auto filter_panel = [&] () -> QWidget* {
      auto value = panel.get();
      if(value == 0) {
        auto body = new QWidget();
        auto body_layout = new QGridLayout(body);
        body_layout->setSpacing(scale_width(5));
        body_layout->setContentsMargins({});
        for(auto i = 0; i < 2; ++i) {
          for(auto j = 0; j < 2; ++j) {
            auto label = make_label("Label:");
            label->setMinimumWidth(scale_width(40));
            body_layout->addWidget(label, i, 2 * j);
            auto text_box = new TextBox();
            text_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            text_box->setMinimumWidth(scale_width(120));
            body_layout->addWidget(text_box, i, 2 * j + 1);
          }
        }
        auto box = new Box(body);
        update_style(*box, [&] (auto& style) {
          style.get(Any()).
            set(BackgroundColor(QColor(0xFFFFFF))).
            set(border(scale_width(1), QColor(Qt::transparent))).
            set(horizontal_padding(scale_width(8))).
            set(vertical_padding(scale_height(8)));
        });
        return box;
      } else if(value == 1) {
        return make_filter_panel();
      } else if(value == 2) {
        return new DateFilterPanel(
          make_local_value_model<DateFilterPanel::DateRange>(
            DateFilterPanel::RelativeDateRange{
              DateFilterPanel::DateUnit::WEEK, 10}));
      }
      return nullptr;
    }();
    auto parent = new QWidget();
    auto popup_box = make_popup_panel(*filter_panel);
    enclose(*parent, *popup_box);
    apply_widget_properties(parent, profile.get_properties());
    auto& overflow_directions =
      get<Qt::Orientations>("overflow_directions", profile.get_properties());
    overflow_directions.connect_changed_signal([=] (auto value) {
      popup_box->set_overflow_directions(value);
    });
    return parent;
  });
  return profile;
}

UiProfile Spire::make_tooltip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("tooltip-text", QString("Tooltip Text")));
  auto profile = UiProfile("Tooltip", properties, [] (auto& profile) {
    auto label = make_label("Hover me!");
    auto& tooltip_text =
      get<QString>("tooltip-text", profile.get_properties());
    auto tooltip = new Tooltip(tooltip_text.get(), label);
    tooltip_text.connect_changed_signal([=] (const auto& text) {
      tooltip->set_label(text);
    });
    return label;
  });
  return profile;
}
