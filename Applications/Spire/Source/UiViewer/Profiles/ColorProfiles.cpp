#include "Spire/UiViewer/Profiles/ColorProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/ColorPicker.hpp"
#include "Spire/Ui/EyeDropper.hpp"
#include "Spire/Ui/HexColorBox.hpp"
#include "Spire/Ui/HighlightBox.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfilePanels.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  QString format_highlight(const HighlightColor& highlight) {
    return highlight.m_background_color.name() + " " +
      highlight.m_text_color.name();
  }

  const auto& get_preset_property() {
    static auto property = define_enum<ColorPicker::Preset>(
      {{"BASIC", ColorPicker::Preset::BASIC},
        {"WITH_TRANSPARENCY", ColorPicker::Preset::WITH_TRANSPARENCY},
        {"ADVANCED", ColorPicker::Preset::ADVANCED}});
    return property;
  }
}

UiProfile Spire::make_color_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("current", QColor(0xF0D109)));
  properties.push_back(make_standard_enum_property("preset",
    get_preset_property()));
  auto profile = UiProfile("ColorBox", properties, [] (auto& profile) {
    auto& preset = get<ColorPicker::Preset>("preset", profile.get_properties());
    auto& current = get<QColor>("current", profile.get_properties());
    auto color_box = new ColorBox(
      std::make_shared<LocalColorModel>(current.get()), preset.get());
    color_box->setFixedSize(scale(100, 26));
    apply_widget_properties(color_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ColorBox::set_read_only, color_box));
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        color_box->get_current()->set(color);
      }
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    color_box->get_current()->connect_update_signal([=] (const auto& current) {
      current_slot(QString("Hex:%1 Alpha:%2").
        arg(current.name()).arg(std::round(current.alphaF() * 100)));
    });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    color_box->connect_submit_signal([=] (const auto& submission) {
      submit_slot(QString("Hex:%1 Alpha:%2").
        arg(submission.name()).arg(std::round(submission.alphaF() * 100)));
    });
    return color_box;
  });
  return profile;
}

UiProfile Spire::make_color_code_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QColor>("current"));
  auto mode_property = define_enum<ColorCodePanel::Mode>(
    {{"HEX", ColorCodePanel::Mode::HEX}, {"RGB", ColorCodePanel::Mode::RGB},
      {"HSB", ColorCodePanel::Mode::HSB}});
  properties.push_back(make_standard_enum_property("mode", mode_property));
  properties.push_back(make_standard_property("alpha_visible", true));
  auto profile = UiProfile("ColorCodePanel", properties, [] (auto& profile) {
    auto panel = new ColorCodePanel();
    apply_widget_properties(panel, profile.get_properties());
    auto& current = get<QColor>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        panel->get_current()->set(color);
      }
    });
    auto& mode = get<ColorCodePanel::Mode>("mode", profile.get_properties());
    mode.connect_changed_signal([=] (auto value) {
      panel->set_mode(value);
    });
    auto& alpha_visible = get<bool>("alpha_visible", profile.get_properties());
    alpha_visible.connect_changed_signal(
      std::bind_front(&ColorCodePanel::set_alpha_visible, panel));
    auto current_slot = profile.make_event_slot<QString>("Current");
    panel->get_current()->connect_update_signal([=] (const auto& current) {
      auto hue = [&] {
        if(current.hueF() < 0) {
          return 0.0;
        }
        return std::round(current.hueF() * 360);
      }();
      current_slot(
        QString("Hex:%1 Hue:%2 Saturation:%3 Brightness:%4 Alpha:%5").
          arg(current.name()).arg(hue).
          arg(std::round(current.saturationF() * 100)).
          arg(std::round(current.valueF() * 100)).
          arg(std::round(current.alphaF() * 100)));
    });
    return panel;
  });
  return profile;
}

UiProfile Spire::make_color_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("current", QColor()));
  properties.push_back(make_standard_enum_property("preset",
    get_preset_property()));
  auto profile = UiProfile("ColorPicker", properties, [] (auto& profile) {
    auto& preset = get<ColorPicker::Preset>("preset", profile.get_properties());
    auto button = make_label_button("ColorPicker");
    auto picker = new ColorPicker(preset.get(), *button);
    auto& current = get<QColor>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        picker->get_current()->set(color);
      }
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    picker->get_current()->connect_update_signal([=] (const auto& current) {
      current_slot(QString("Hex:%1 Alpha:%2").
        arg(current.name()).arg(std::round(current.alphaF() * 100)));
    });
    button->connect_click_signal([=] { picker->show(); });
    return button;
  });
  return profile;
}

UiProfile Spire::make_eye_dropper_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("popup_window", false));
  auto profile = UiProfile("EyeDropper", properties, [] (auto& profile) {
    auto create_eye_dropper = [] (QWidget* parent, UiProfile& profile) {
      auto eye_dropper = new EyeDropper(parent);
      eye_dropper->show();
      eye_dropper->get_current()->connect_update_signal(
        profile.make_event_slot<QColor>("Current"));
      eye_dropper->connect_submit_signal(
        profile.make_event_slot<QColor>("Submit"));
      eye_dropper->connect_reject_signal(
        profile.make_event_slot<QColor>("Reject"));
    };
    auto button = make_label_button("EyeDropper");
    auto& popup_window = get<bool>("popup_window", profile.get_properties());
    button->connect_click_signal([&, button] {
      if(popup_window.get()) {
        auto body = create_panel_body("EyeDropper");
        auto panel = new OverlayPanel(*body, *button);
        auto child_button = body->findChild<Button*>();
        child_button->connect_click_signal([=, &profile] {
          create_eye_dropper(child_button, profile);
        });
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->set_closed_on_focus_out(true);
        panel->set_is_draggable(true);
        panel->set_positioning(OverlayPanel::Positioning::NONE);
        panel->show();
      } else {
        create_eye_dropper(button, profile);
      }
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_hex_color_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QColor>("current"));
  properties.push_back(make_standard_property<QColor>("rejected", Qt::red));
  auto profile = UiProfile("HexColorBox", properties, [] (auto& profile) {
    auto& rejected = get<QColor>("rejected", profile.get_properties());
    auto model = make_validated_value_model<QColor>([&] (QColor value) {
      if(value == rejected.get()) {
        return QValidator::State::Invalid;
      }
      return QValidator::State::Acceptable;
    });
    auto box = new HexColorBox(model);
    box->setFixedWidth(scale_width(120));
    apply_widget_properties(box, profile.get_properties());
    link(box->get_current(), get<QColor>("current", profile.get_properties()));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QColor>("Current"));
    box->connect_submit_signal(profile.make_event_slot<QColor>("Submit"));
    box->connect_reject_signal(profile.make_event_slot<QColor>("Reject"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_highlight_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFFF)));
  properties.push_back(make_standard_property("text_color", QColor(Qt::black)));
  populate_font_properties(properties, "font");
  auto profile = UiProfile("HighlightBox", properties, [] (auto& profile) {
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    auto highlight_box = new HighlightBox(
      std::make_shared<LocalHighlightColorModel>(
        HighlightColor{background_color.get(), text_color.get()}));
    highlight_box->setFixedSize(scale(100, 26));
    apply_widget_properties(highlight_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&HighlightBox::set_read_only, highlight_box));
    background_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = highlight_box->get_current()->get();
      if(highlight.m_background_color.name() != color.name()) {
        highlight.m_background_color = color;
        highlight_box->get_current()->set(highlight);
      }
    });
    text_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = highlight_box->get_current()->get();
      if(highlight.m_text_color.name() != color.name()) {
        highlight.m_text_color = color;
        highlight_box->get_current()->set(highlight);
      }
    });
    auto& font = get<QFont>("font", profile.get_properties());
    font.connect_changed_signal([=] (const auto& value) {
      update_style(*highlight_box, [&] (auto& style) {
        style.get(Any() > is_a<TextBox>()).set(Font(value));
      });
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    highlight_box->get_current()->connect_update_signal(
      [=, &background_color, &text_color] (const auto& highlight) {
        if(background_color.get().name() !=
            highlight.m_background_color.name()) {
          background_color.set(highlight.m_background_color);
        }
        if(text_color.get().name() != highlight.m_text_color.name()) {
          text_color.set(highlight.m_text_color);
        }
        current_slot(format_highlight(highlight));
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    highlight_box->connect_submit_signal([=] (const auto& submission) {
      submit_slot(format_highlight(submission));
    });
    return highlight_box;
  });
  return profile;
}

UiProfile Spire::make_highlight_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFC4)));
  properties.push_back(make_standard_property("text_color", QColor(0x521C00)));
  auto profile = UiProfile("HighlightPicker", properties, [] (auto& profile) {
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    auto button = make_label_button("HighlightPicker");
    auto picker = new HighlightPicker(make_local_value_model(
      HighlightPicker::Highlight{background_color.get(), text_color.get()}),
      *button);
    background_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = picker->get_current()->get();
      if(highlight.m_background_color.name() != color.name()) {
        highlight.m_background_color = color;
        picker->get_current()->set(highlight);
      }
      update_style(*button, [&] (auto& style) {
        style.get(Any() > Body()).set(BackgroundColor(color));
      });
    });
    text_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = picker->get_current()->get();
      if(highlight.m_text_color.name() != color.name()) {
        highlight.m_text_color = color;
        picker->get_current()->set(highlight);
      }
      update_style(*button, [&] (auto& style) {
        style.get(Any() > Body()).set(TextColor(color));
      });
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    picker->get_current()->connect_update_signal(
      [=, &background_color, &text_color] (const auto& highlight) {
        if(background_color.get().name() !=
            highlight.m_background_color.name()) {
          background_color.set(highlight.m_background_color);
        }
        if(text_color.get().name() != highlight.m_text_color.name()) {
          text_color.set(highlight.m_text_color);
        }
        current_slot(format_highlight(highlight));
      });
    button->connect_click_signal([=] { picker->show(); });
    return button;
  });
  return profile;
}

UiProfile Spire::make_highlight_swatch_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFC4)));
  properties.push_back(make_standard_property("text_color", QColor(0x521C00)));
  auto profile = UiProfile("HighlightSwatch", properties, [] (auto& profile) {
    auto swatch = new HighlightSwatch();
    apply_widget_properties(swatch, profile.get_properties());
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    background_color.connect_changed_signal([=] (auto color) {
      auto highlight = swatch->get_current()->get();
      highlight.m_background_color = color;
      swatch->get_current()->set(highlight);
    });
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    text_color.connect_changed_signal([=] (auto color) {
      auto highlight = swatch->get_current()->get();
      highlight.m_text_color = color;
      swatch->get_current()->set(highlight);
    });
    return swatch;
  });
  return profile;
}
