#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include <QLabel>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_bool_property("display_warning"));
  auto profile = UiProfile(QString::fromUtf8("Box"), properties,
    [] (auto& profile) {
      auto box = new Box(nullptr);
      box->resize(scale(100, 100));
      auto style = StyleSheet();
      style.get(Any()).
        set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
        set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
        set(horizontal_padding(scale_width(8)));
      style.get(Hover() || Focus()).
        set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
      style.get(Disabled()).
        set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
        set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
      set_style(*box, std::move(style));
      apply_widget_properties(box, profile.get_properties());
      auto& warning = get<bool>("display_warning", profile.get_properties());
      warning.connect_changed_signal([&warning, box] (auto is_playing_warning) {
        if(is_playing_warning) {
          display_warning_indicator(*box);
          warning.set(false);
        }
      });
      return box;
    });
  return profile;
}

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

UiProfile Spire::make_decimal_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("current",
    QString::fromUtf8("1")));
  properties.push_back(make_standard_qstring_property("minimum",
    QString::fromUtf8("-100")));
  properties.push_back(make_standard_qstring_property("maximum",
    QString::fromUtf8("100")));
  properties.push_back(make_standard_int_property("decimal_places", 2));
  properties.push_back(make_standard_int_property("leading_zeros", 0));
  properties.push_back(make_standard_int_property("trailing_zeros", 2));
  properties.push_back(make_standard_qstring_property("default_increment",
    QString::fromUtf8("1")));
  properties.push_back(make_standard_qstring_property("alt_increment",
    QString::fromUtf8("5")));
  properties.push_back(make_standard_qstring_property("ctrl_increment",
    QString::fromUtf8("10")));
  properties.push_back(make_standard_qstring_property("shift_increment",
    QString::fromUtf8("20")));
  properties.push_back(make_standard_qstring_property("placeholder"));
  properties.push_back(make_standard_bool_property("read_only", false));
  properties.push_back(make_standard_bool_property("buttons_visible", true));
  properties.push_back(make_standard_bool_property("is_warning_displayed",
    true));
  auto profile = UiProfile(QString::fromUtf8("DecimalBox"), properties,
    [] (auto& profile) {
      auto parse_decimal = [] (auto decimal) ->
          std::optional<DecimalBox::Decimal> {
        try {
          return DecimalBox::Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto model =
        std::make_shared<LocalScalarValueModel<DecimalBox::Decimal>>();
      auto& minimum = get<QString>("minimum", profile.get_properties());
      minimum.connect_changed_signal([=] (const auto& value) {
        if(auto minimum = parse_decimal(value)) {
          model->set_minimum(*minimum);
        }
      });
      auto& maximum = get<QString>("maximum", profile.get_properties());
      maximum.connect_changed_signal([=] (const auto& value) {
        if(auto maximum = parse_decimal(value)) {
          model->set_maximum(*maximum);
        }
      });
      auto& decimal_places = get<int>("decimal_places",
        profile.get_properties());
      decimal_places.connect_changed_signal([=] (auto value) {
        model->set_increment(pow(DecimalBox::Decimal(10), -value));
      });
      auto& default_increment = get<QString>("default_increment",
        profile.get_properties());
      auto& alt_increment = get<QString>("alt_increment",
        profile.get_properties());
      auto& ctrl_increment = get<QString>("ctrl_increment",
        profile.get_properties());
      auto& shift_increment = get<QString>("shift_increment",
        profile.get_properties());
      auto modifiers = QHash<Qt::KeyboardModifier, DecimalBox::Decimal>(
        {{Qt::NoModifier, *parse_decimal(default_increment.get())},
         {Qt::AltModifier, *parse_decimal(alt_increment.get())},
         {Qt::ControlModifier, *parse_decimal(ctrl_increment.get())},
         {Qt::ShiftModifier, *parse_decimal(shift_increment.get())}});
      auto decimal_box = new DecimalBox(model, modifiers);
      apply_widget_properties(decimal_box, profile.get_properties());
      auto& leading_zeros = get<int>("leading_zeros", profile.get_properties());
      leading_zeros.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        style.get(Any()).set(LeadingZeros(value));
        set_style(*decimal_box, std::move(style));
      });
      auto& trailing_zeros = get<int>("trailing_zeros",
        profile.get_properties());
      trailing_zeros.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        style.get(Any()).set(TrailingZeros(value));
        set_style(*decimal_box, std::move(style));
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& value) {
        if(auto decimal = parse_decimal(value)) {
          if(decimal_box->get_model()->get_current().compare(*decimal) != 0) {
            decimal_box->get_model()->set_current(*decimal);
          }
        }
      });
      auto current_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Current"));
      decimal_box->get_model()->connect_current_signal(
        [=] (const DecimalBox::Decimal& current) {
          current_slot(QString::fromStdString(
            current.str(DecimalBox::PRECISION, std::ios_base::dec)));
        });
      auto submit_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Submit"));
      decimal_box->connect_submit_signal(
        [=] (const DecimalBox::Decimal& submission) {
          submit_slot(QString::fromStdString(
            submission.str(DecimalBox::PRECISION, std::ios_base::dec)));
        });
      auto reject_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Reject"));
      decimal_box->connect_reject_signal(
        [=] (const DecimalBox::Decimal& value) {
          reject_slot(QString::fromStdString(
            value.str(DecimalBox::PRECISION, std::ios_base::dec)));
        });
      auto& placeholder = get<QString>("placeholder",
        profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& placeholder) {
        decimal_box->set_placeholder(placeholder);
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto value) {
        decimal_box->set_read_only(value);
      });
      auto& buttons_visible = get<bool>("buttons_visible",
        profile.get_properties());
      buttons_visible.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(Any() > is_a<Button>()).get_block().remove<Visibility>();
        } else {
          style.get(Any() > is_a<Button>()).set(
            Visibility(VisibilityOption::NONE));
        }
        set_style(*decimal_box, std::move(style));
      });
      auto& is_warning_displayed = get<bool>("is_warning_displayed",
        profile.get_properties());
      is_warning_displayed.connect_changed_signal([=] (auto value) {
        decimal_box->set_warning_displayed(value);
      });
      return decimal_box;
    });
  return profile;
}

UiProfile Spire::make_flat_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("label",
    QString::fromUtf8("Click me!")));
  auto profile = UiProfile(QString::fromUtf8("LabelButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_label_button(label.get());
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(profile.make_event_slot(
        QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("IconButton"), properties,
    [] (auto& profile) {
      auto button = make_icon_button(imageFromSvg(":/Icons/demo.svg",
        scale(26, 26)));
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(profile.make_event_slot(
        QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_integer_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_int_property("current", 1));
  properties.push_back(make_standard_int_property("minimum", -100));
  properties.push_back(make_standard_int_property("maximum", 100));
  properties.push_back(make_standard_int_property("default_increment", 1));
  properties.push_back(make_standard_int_property("alt_increment", 5));
  properties.push_back(make_standard_int_property("ctrl_increment", 10));
  properties.push_back(make_standard_int_property("shift_increment", 20));
  properties.push_back(make_standard_qstring_property("placeholder"));
  properties.push_back(make_standard_bool_property("read_only", false));
  properties.push_back(make_standard_bool_property("buttons_visible", true));
  properties.push_back(make_standard_bool_property("is_warning_displayed",
    true));
  auto profile = UiProfile(QString::fromUtf8("IntegerBox"), properties,
    [] (auto& profile) {
      auto model = std::make_shared<LocalIntegerModel>();
      auto& minimum = get<int>("minimum", profile.get_properties());
      minimum.connect_changed_signal([=] (auto value) {
        model->set_minimum(value);
      });
      auto& maximum = get<int>("maximum", profile.get_properties());
      maximum.connect_changed_signal([=] (auto value) {
        model->set_maximum(value);
      });
      auto& default_increment = get<int>("default_increment",
        profile.get_properties());
      auto& alt_increment = get<int>("alt_increment", profile.get_properties());
      auto& ctrl_increment = get<int>("ctrl_increment",
        profile.get_properties());
      auto& shift_increment = get<int>("shift_increment",
        profile.get_properties());
      auto modifiers = QHash<Qt::KeyboardModifier, int>(
        {{Qt::NoModifier, default_increment.get()},
         {Qt::AltModifier, alt_increment.get()},
         {Qt::ControlModifier, ctrl_increment.get()},
         {Qt::ShiftModifier, shift_increment.get()}});
      auto integer_box = new IntegerBox(model, modifiers);
      apply_widget_properties(integer_box, profile.get_properties());
      auto& current = get<int>("current", profile.get_properties());
      current.connect_changed_signal([=] (auto value) {
        if(integer_box->get_model()->get_current() != value) {
          integer_box->get_model()->set_current(value);
        }
      });
      integer_box->get_model()->connect_current_signal(
        profile.make_event_slot<int>(QString::fromUtf8("Current")));
      integer_box->connect_submit_signal(
        profile.make_event_slot<int>(QString::fromUtf8("Submit")));
      integer_box->connect_reject_signal(
        profile.make_event_slot<int>(QString::fromUtf8("Reject")));
      auto& placeholder = get<QString>("placeholder",
        profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& placeholder) {
        integer_box->set_placeholder(placeholder);
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto value) {
        integer_box->set_read_only(value);
      });
      auto& buttons_visible = get<bool>("buttons_visible",
        profile.get_properties());
      buttons_visible.connect_changed_signal([=] (auto value) {
        auto style = get_style(*integer_box);
        if(value) {
          style.get(Any() > is_a<Button>()).get_block().remove<Visibility>();
        } else {
          style.get(Any() > is_a<Button>()).set(
            Visibility(VisibilityOption::NONE));
        }
        set_style(*integer_box, std::move(style));
      });
      auto& is_warning_displayed = get<bool>("is_warning_displayed",
        profile.get_properties());
      is_warning_displayed.connect_changed_signal([=] (auto value) {
        integer_box->set_warning_displayed(value);
      });
      return integer_box;
    });
  return profile;
}

UiProfile Spire::make_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("ListItem"), properties,
    [] (auto& profile) {
      auto text_box = new TextBox("Test Component");
      text_box->setAttribute(Qt::WA_TranslucentBackground);
      text_box->set_read_only(true);
      text_box->setDisabled(true);
      auto text_box_style = text_box->get_style();
      text_box_style.get(Disabled()).set(TextColor(QColor::fromRgb(0, 0, 0)));
      text_box->set_style(text_box_style);
      auto list_item = new ListItem(text_box);
      apply_widget_properties(list_item, profile.get_properties());
      list_item->connect_current_signal(profile.make_event_slot(
        QString::fromUtf8("Current")));
      list_item->connect_submit_signal(profile.make_event_slot(
        QString::fromUtf8("Submit")));
      return list_item;
    });
  return profile;
}

UiProfile Spire::make_scroll_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_bool_property("enabled", true));
  properties.push_back(make_standard_bool_property("vertical", true));
  properties.push_back(make_standard_int_property("start-range", 0));
  properties.push_back(make_standard_int_property("end-range", 1000));
  properties.push_back(make_standard_int_property("page-size", 100));
  properties.push_back(make_standard_int_property("line-size", 10));
  properties.push_back(make_standard_int_property("position", 0));
  properties.push_back(make_standard_int_property("thumb-min-size", 50));
  auto profile = UiProfile(QString::fromUtf8("ScrollBar"), properties,
    [] (auto& profile) {
      auto& vertical = get<bool>("vertical", profile.get_properties());
      auto orientation = [&] {
        if(vertical.get()) {
          return Qt::Vertical;
        } else {
          return Qt::Horizontal;
        }
      }();
      auto scroll_bar = new ScrollBar(orientation);
      if(orientation == Qt::Vertical) {
        scroll_bar->resize(scale_width(13), scale_height(200));
      } else {
        scroll_bar->resize(scale_width(200), scale_height(13));
      }
      auto& enabled = get<bool>("enabled", profile.get_properties());
      enabled.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->setEnabled(value);
      });
      auto& start_range = get<int>("start-range", profile.get_properties());
      start_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_start = value;
        scroll_bar->set_range(range);
      });
      auto& end_range = get<int>("end-range", profile.get_properties());
      end_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_end = value;
        scroll_bar->set_range(range);
      });
      auto& page_size = get<int>("page-size", profile.get_properties());
      page_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_page_size(value);
      });
      auto& line_size = get<int>("line-size", profile.get_properties());
      line_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_line_size(value);
      });
      auto& position = get<int>("position", profile.get_properties());
      position.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_position(value);
      });
      auto& thumb_min_size = get<int>("thumb-min-size", profile.get_properties());
      thumb_min_size.connect_changed_signal([scroll_bar] (auto value) {
        auto min_size = [=] {
          if(scroll_bar->get_orientation() == Qt::Vertical) {
            return scale_height(value);
          } else {
            return scale_width(value);
          }
        }();
        scroll_bar->set_thumb_min_size(min_size);
      });
      scroll_bar->connect_position_signal(profile.make_event_slot<int>(
        QString::fromUtf8("Position")));
      return scroll_bar;
    });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_bool_property("read_only"));
  properties.push_back(make_standard_qstring_property("current"));
  properties.push_back(make_standard_qstring_property("placeholder"));
  properties.push_back(make_standard_bool_property("display_warning"));
  auto profile = UiProfile(QString::fromUtf8("TextBox"), properties,
    [] (auto& profile) {
      auto text_box = new TextBox();
      apply_widget_properties(text_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([text_box] (auto is_read_only) {
        text_box->set_read_only(is_read_only);
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& current) {
        if(text_box->get_model()->get_current() != current) {
          text_box->get_model()->set_current(current);
        }
      });
      auto& placeholder = get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([text_box] (const auto& text) {
        text_box->set_placeholder(text);
      });
      auto& warning = get<bool>("display_warning", profile.get_properties());
      warning.connect_changed_signal(
        [&warning, text_box] (auto is_playing_warning) {
          if(is_playing_warning) {
            display_warning_indicator(*text_box);
            warning.set(false);
          }
        });
      text_box->get_model()->connect_current_signal([&] (const auto& value) {
        current.set(value);
      });
      text_box->get_model()->connect_current_signal(
        profile.make_event_slot<QString>(QString::fromUtf8("Current")));
      text_box->connect_submit_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Submit")));
      text_box->connect_reject_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Reject")));
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
