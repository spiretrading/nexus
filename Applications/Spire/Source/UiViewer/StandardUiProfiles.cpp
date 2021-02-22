#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include <QLabel>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
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
      label.connect_changed_signal([=] (const auto& value) {
        checkbox->setText(value);
      });
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
  properties.push_back(make_standard_qcolor_property("background-color"));
  properties.push_back(make_standard_qcolor_property("border-color"));
  properties.push_back(make_standard_qcolor_property("text-color"));
  properties.push_back(make_standard_int_property("font-size"));
  properties.push_back(make_standard_int_property("font-weight"));
  properties.push_back(make_standard_qcolor_property("hover-background-color"));
  properties.push_back(make_standard_qcolor_property("hover-border-color"));
  properties.push_back(make_standard_qcolor_property("focus-background-color"));
  properties.push_back(make_standard_qcolor_property("focus-border-color"));
  properties.push_back(make_standard_qcolor_property("disabled-background-color"));
  properties.push_back(make_standard_qcolor_property("disabled-border-color"));
  properties.push_back(make_standard_int_property("hover-width"));
  properties.push_back(make_standard_int_property("hover-height"));
  properties.push_back(make_standard_qstring_property("focus-relative-width"));
  properties.push_back(make_standard_qstring_property("focus-relative-height"));
  auto profile = UiProfile(QString::fromUtf8("FlatButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_button(label.get());
      label.connect_changed_signal([=] (const auto& value) {
        button->set_text(value);
      });
      auto& enabled = get<bool>("enabled", profile.get_properties());
      enabled.connect_changed_signal([=] (auto value) {
        button->setEnabled(value);
      });
      auto initialize_color_property = [&] (const auto& property_name,
          auto get_style_pointer, auto set_style_pointer, auto member_pointer) {
        auto& property = get<QColor>(property_name, profile.get_properties());
        property.set((button->*get_style_pointer)().*member_pointer);
        property.connect_changed_signal([=] (const auto& color) {
          auto style = (button->*get_style_pointer)();
          style.*member_pointer = color;
          (button->*set_style_pointer)(style);
        });
      };
      initialize_color_property("background-color", &Button::get_style,
        &Button::set_style, &Box::Style::m_background_color);
      initialize_color_property("border-color", &Button::get_style,
        &Button::set_style, &Box::Style::m_border_color);
      initialize_color_property("hover-background-color",
        &Button::get_hover_style, &Button::set_hover_style,
        &Box::Style::m_background_color);
      initialize_color_property("hover-border-color", &Button::get_hover_style,
        &Button::set_hover_style, &Box::Style::m_border_color);
      initialize_color_property("focus-background-color",
        &Button::get_focus_style, &Button::set_focus_style,
        &Box::Style::m_background_color);
      initialize_color_property("focus-border-color", &Button::get_focus_style,
        &Button::set_focus_style, &Box::Style::m_border_color);
      initialize_color_property("disabled-background-color",
        &Button::get_disabled_style, &Button::set_disabled_style,
        &Box::Style::m_background_color);
      initialize_color_property("disabled-border-color",
        &Button::get_disabled_style, &Button::set_disabled_style,
        &Box::Style::m_border_color);
      auto text_box = button->get_text_box();
      auto text_styles = text_box->get_styles();
      auto& text_color = get<QColor>("text-color", profile.get_properties());
      text_color.set(*(text_styles.m_style.m_text_color));
      text_color.connect_changed_signal([=] (const auto& color) {
        auto styles = text_box->get_styles();
        styles.m_style.m_text_color = color;
        text_box->set_styles(styles);
      });
      auto font = *(text_styles.m_style.m_font);
      auto& font_size = get<int>("font-size", profile.get_properties());
      font_size.set(unscale_width(font.pixelSize()));
      font_size.connect_changed_signal([text_box] (auto value) {
        auto styles = text_box->get_styles();
        styles.m_style.m_font->setPixelSize(scale_width(value));
        text_box->set_styles(styles);
      });
      auto& font_weight = get<int>("font-weight", profile.get_properties());
      font_weight.set(font.weight());
      font_weight.connect_changed_signal([text_box] (auto value) {
        if(value < 0 || value > 99)
          return;
        auto styles = text_box->get_styles();
        styles.m_style.m_font->setWeight(value);
        text_box->set_styles(styles);
      });
      auto initialize_size_property = [&] (const auto& property_name,
          auto get_style_pointer, auto set_style_pointer, auto get_pointer,
          auto set_pointer, auto scale_pointer, auto unscale_pointer) {
        auto& property = get<int>(property_name, profile.get_properties());
        auto style = (button->*get_style_pointer)();
        if(!style.m_size) {
          style.m_size = button->get_style().m_size;
          (button->*set_style_pointer)(style);
        }
        property.set((*unscale_pointer)((boost::get<QSize>(
          style.m_size.get_ptr())->*get_pointer)()));
        property.connect_changed_signal([=] (const auto& value) {
          if(value != 0) {
            auto style = (button->*get_style_pointer)();
            if((*unscale_pointer)((boost::get<QSize>(
                style.m_size.get_ptr())->*get_pointer)()) != value) {
              (boost::get<QSize>(style.m_size.get_ptr())->*set_pointer)
                ((*scale_pointer)(value));
              (button->*set_style_pointer)(style);
            }
          }
        });
      };
      initialize_size_property("width", &Button::get_style, &Button::set_style,
        &QSize::width, &QSize::setWidth, &scale_width, &unscale_width);
      initialize_size_property("height", &Button::get_style, &Button::set_style,
        &QSize::height, &QSize::setHeight, &scale_height, &unscale_height);
      initialize_size_property("hover-width", &Button::get_hover_style,
        &Button::set_hover_style, &QSize::width, &QSize::setWidth, &scale_width,
        &unscale_width);
      initialize_size_property("hover-height", &Button::get_hover_style,
        &Button::set_hover_style, &QSize::height, &QSize::setHeight,
        &scale_height, &unscale_height);
      auto initialize_relative_size_property = [&] (const auto& property_name,
          auto get_style_pointer, auto set_style_pointer, auto get_pointer,
          auto set_pointer) {
        auto& property = get<QString>(property_name, profile.get_properties());
        auto style = (button->*get_style_pointer)();
        style.m_size = QSizeF(-1.0, -1.0);
        (button->*set_style_pointer)(style);
        property.set(QString().setNum((boost::get<QSizeF>(
          style.m_size.get_ptr())->*get_pointer)(), 'f', 2));
        property.connect_changed_signal([=] (const auto& text) {
          auto ok = true;
          auto value = text.toFloat(&ok);
          if(ok) {
            auto style = (button->*get_style_pointer)();
            (boost::get<QSizeF>(style.m_size.get_ptr())->*set_pointer)(value);
            (button->*set_style_pointer)(style);
          }
        });
      };
      initialize_relative_size_property("focus-relative-width",
        &Button::get_focus_style, &Button::set_focus_style, &QSizeF::width,
        &QSizeF::setWidth);
      initialize_relative_size_property("focus-relative-height",
        &Button::get_focus_style, &Button::set_focus_style, &QSizeF::height,
        &QSizeF::setHeight);
      button->connect_clicked_signal(profile.make_event_slot(
        QString::fromUtf8("ClickedSignal")));
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
  properties.push_back(make_standard_qcolor_property("blur-color"));
  properties.push_back(make_standard_qcolor_property("checked-color"));
  properties.push_back(make_standard_qcolor_property("checked-blur-color"));
  properties.push_back(make_standard_qcolor_property("checked-hover-color"));
  properties.push_back(make_standard_qcolor_property("default-color"));
  properties.push_back(make_standard_qcolor_property("disabled-color"));
  properties.push_back(make_standard_qcolor_property("hover-color"));
  properties.push_back(make_standard_qcolor_property("default-bg-color"));
  properties.push_back(make_standard_qcolor_property("hover-bg-color"));
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
      auto initialize_color_property = [&] (const auto& property_name,
          auto member_pointer) {
        auto& property = get<QColor>(property_name, profile.get_properties());
        property.set(button->get_style().*member_pointer);
        property.connect_changed_signal([=] (const auto& color) {
          auto style = button->get_style();
          style.*member_pointer = color;
          button->set_style(style);
        });
      };
      initialize_color_property("blur-color", &IconButton::Style::m_blur_color);
      initialize_color_property("checked-color",
        &IconButton::Style::m_checked_color);
      initialize_color_property("checked-blur-color",
        &IconButton::Style::m_checked_blur_color);
      initialize_color_property("checked-hover-color",
        &IconButton::Style::m_checked_hovered_color);
      initialize_color_property("default-color",
        &IconButton::Style::m_default_color);
      initialize_color_property("disabled-color",
        &IconButton::Style::m_disabled_color);
      initialize_color_property("hover-color", &IconButton::Style::m_hover_color);
      initialize_color_property("default-bg-color",
        &IconButton::Style::m_default_background_color);
      initialize_color_property("hover-bg-color",
        &IconButton::Style::m_hover_background_color);
      return button;
    });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_bool_property("read_only"));
  properties.push_back(make_standard_int_property("font_size"));
  properties.push_back(make_standard_int_property("font_weight"));
  properties.push_back(make_standard_bool_property("align_left"));
  properties.push_back(make_standard_bool_property("align_right"));
  properties.push_back(make_standard_bool_property("align_center"));
  properties.push_back(make_standard_int_property("left_padding"));
  properties.push_back(make_standard_int_property("right_padding"));
  properties.push_back(make_standard_qstring_property("submission"));
  properties.push_back(make_standard_bool_property("playing_warning"));
  auto profile = UiProfile(QString::fromUtf8("TextBox"), properties,
    [] (auto& profile) {
      auto text_box = new TextBox();
      apply_widget_properties(text_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([text_box] (auto is_read_only) {
        text_box->setReadOnly(is_read_only);
      });
      auto text_styles = text_box->get_styles();
      auto font = *(text_styles.m_style.m_font);
      auto& font_size = get<int>("font_size", profile.get_properties());
      font_size.set(unscale_width(font.pixelSize()));
      font_size.connect_changed_signal([text_box] (auto value) {
        auto styles = text_box->get_styles();
        styles.m_style.m_font->setPixelSize(scale_width(value));
        text_box->set_styles(styles);
      });
      auto& font_weight = get<int>("font_weight", profile.get_properties());
      font_weight.set(font.weight());
      font_weight.connect_changed_signal([text_box] (auto value) {
        if(value < 0 || value > 99)
          return;
        auto styles = text_box->get_styles();
        styles.m_style.m_font->setWeight(value);
        text_box->set_styles(styles);
      });
      auto& align_left = get<bool>("align_left", profile.get_properties());
      auto& align_right = get<bool>("align_right", profile.get_properties());
      auto& align_center = get<bool>("align_center", profile.get_properties());
      align_left.connect_changed_signal([&, text_box] (auto is_align_left) {
        if(is_align_left) {
          text_box->setAlignment(Qt::AlignLeft);
          align_right.set(false);
          align_center.set(false);
        }
      });
      align_right.connect_changed_signal([&, text_box] (auto is_align_right) {
        if(is_align_right) {
          text_box->setAlignment(Qt::AlignRight);
          align_left.set(false);
          align_center.set(false);
        }
      });
      align_center.connect_changed_signal([&, text_box] (auto is_align_center) {
        if(is_align_center) {
          text_box->setAlignment(Qt::AlignCenter);
          align_left.set(false);
          align_right.set(false);
        }
      });
      align_left.set(true);
      auto& left_padding = get<int>("left_padding", profile.get_properties());
      left_padding.set(unscale_width(text_styles.m_style.m_paddings->left()));
      left_padding.connect_changed_signal([text_box] (auto value) {
        auto styles = text_box->get_styles();
        styles.m_style.m_paddings->setLeft(scale_width(value));
        text_box->set_styles(styles);
      });
      auto& right_padding = get<int>("right_padding", profile.get_properties());
      right_padding.set(unscale_width(text_styles.m_style.m_paddings->right()));
      right_padding.connect_changed_signal([text_box] (auto value) {
        auto styles = text_box->get_styles();
        styles.m_style.m_paddings->setRight(scale_width(value));
        text_box->set_styles(styles);
      });
      auto& submission = get<QString>("submission", profile.get_properties());
      submission.connect_changed_signal([text_box] (const auto& text) {
        text_box->set_text(text);
      });
      auto& warning = get<bool>("playing_warning", profile.get_properties());
      warning.connect_changed_signal([&warning, text_box]
          (auto is_playing_warning) {
        if(is_playing_warning) {
          text_box->play_warning();
          warning.set(false);
        }
      });
      text_box->connect_current_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Current")));
      text_box->connect_submit_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Submit")));
      return text_box;
    });
  return profile;
}

UiProfile Spire::make_tooltip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("tooltip-text",
    QString::fromUtf8("Tooltip Text")));
  auto profile = UiProfile(QString::fromUtf8("Tooltip"), properties,
    [] (auto& profile) {
      auto label = new QLabel("Hover me!");
      label->setAttribute(Qt::WA_Hover);
      label->setFocusPolicy(Qt::StrongFocus);
      label->resize(scale(100, 28));
      label->setStyleSheet(QString(R"(
        QLabel {
          background-color: #684BC7;
          color: white;
          qproperty-alignment: AlignCenter;
        }

        QLabel:focus {
          border: %1px solid #000000;
        }

        QLabel:disabled {
          background-color: #F5F5F5;
          color: #C8C8C8;
        })").arg(scale_width(2)));
      apply_widget_properties(label, profile.get_properties());
      auto& tooltip_text = get<QString>("tooltip-text",
        profile.get_properties());
      auto tooltip = make_text_tooltip(tooltip_text.get(), label);
      return label;
    });
  return profile;
}
