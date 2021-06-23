#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include <QImageReader>
#include <QLabel>
#include <QPointer>
#include <QRandomGenerator>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  template<typename B>
  auto setup_decimal_box_profile(UiProfile& profile) {
    using Type = std::decay_t<decltype(*std::declval<B>().get_model())>::Scalar;
    auto& width = get<int>("width", profile.get_properties());
    width.set(scale_width(100));
    auto model = std::make_shared<LocalScalarValueModel<optional<Type>>>();
    auto& minimum = get<Type>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      model->set_minimum(value);
    });
    auto& maximum = get<Type>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      model->set_maximum(value);
    });
    auto& default_increment =
      get<Type>("default_increment", profile.get_properties());
    auto& alt_increment = get<Type>("alt_increment", profile.get_properties());
    auto& ctrl_increment =
      get<Type>("ctrl_increment", profile.get_properties());
    auto& shift_increment =
      get<Type>("shift_increment", profile.get_properties());
    auto modifiers = QHash<Qt::KeyboardModifier, Type>(
      {{Qt::NoModifier, default_increment.get()},
        {Qt::AltModifier, alt_increment.get()},
        {Qt::ControlModifier, ctrl_increment.get()},
        {Qt::ShiftModifier, shift_increment.get()}});
    auto box = new B(std::move(model), std::move(modifiers));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(box->get_model()->get_current() != value) {
        box->get_model()->set_current(value);
      }
    });
    box->get_model()->connect_current_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Current")));
    box->connect_submit_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Submit")));
    box->connect_reject_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Reject")));
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto value) {
      box->set_read_only(value);
    });
    auto& buttons_visible =
      get<bool>("buttons_visible", profile.get_properties());
    buttons_visible.connect_changed_signal([=] (auto value) {
      auto style = get_style(*box);
      if(value) {
        style.get(Any() > is_a<Button>()).get_block().remove<Visibility>();
      } else {
        style.get(Any() > is_a<Button>()).set(
          Visibility(VisibilityOption::NONE));
      }
      set_style(*box, std::move(style));
    });
    return box;
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      T default_increment) {
    using Type = T;
    properties.push_back(make_standard_property("current", Type(1)));
    properties.push_back(make_standard_property("minimum", Type(-100)));
    properties.push_back(make_standard_property("maximum", Type(100)));
    properties.push_back(
      make_standard_property("default_increment", default_increment));
    properties.push_back(
      make_standard_property("alt_increment", 5 * default_increment));
    properties.push_back(
      make_standard_property("ctrl_increment", 10 * default_increment));
    properties.push_back(
      make_standard_property("shift_increment", 20 * default_increment));
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    properties.push_back(make_standard_property("buttons_visible", true));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    populate_decimal_box_properties<T>(properties, 1);
  }
}

UiProfile Spire::make_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("border-size", 1));
  properties.push_back(make_standard_property("border-radius", 0));
  auto profile = UiProfile(QString::fromUtf8("Box"), properties,
    [] (auto& profile) {
      auto box = new Box(nullptr);
      box->resize(scale(100, 100));
      auto& border_size = get<int>("border-size", profile.get_properties());
      auto& border_radius = get<int>("border-radius",
        profile.get_properties());
      auto style = StyleSheet();
      style.get(Any()).
        set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
        set(border(scale_width(border_size.get()),
          QColor::fromRgb(0xC8, 0xC8, 0xC8))).
        set(Styles::border_radius(scale_width(border_radius.get()))).
        set(horizontal_padding(scale_width(8)));
      style.get(Hover() || Focus()).
        set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
      style.get(Disabled()).
        set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
        set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
      set_style(*box, std::move(style));
      border_size.connect_changed_signal([&, box] (auto size) {
        auto style = get_style(*box);
        style.get(Any()).set(Styles::border_size(scale_width(
          border_size.get())));
        set_style(*box, style);
      });
      border_radius.connect_changed_signal([&, box] (auto radius) {
        auto style = get_style(*box);
        style.get(Any()).set(Styles::border_radius(scale_width(
          border_radius.get())));
        set_style(*box, style);
      });
      apply_widget_properties(box, profile.get_properties());
      return box;
    });
  return profile;
}

UiProfile Spire::make_check_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("checked"));
  properties.push_back(
    make_standard_property("label", QString::fromUtf8("Click me!")));
  properties.push_back(make_standard_property<bool>("read-only"));
  properties.push_back(make_standard_property("left-to-right", true));
  auto profile = UiProfile(QString::fromUtf8("CheckBox"), properties,
    [] (auto& profile) {
      auto check_box = new CheckBox();
      auto& label = get<QString>("label", profile.get_properties());
      check_box->set_label(label.get());
      apply_widget_properties(check_box, profile.get_properties());
      label.connect_changed_signal([=] (const auto& value) {
        check_box->set_label(value);
      });
      auto& checked = get<bool>("checked", profile.get_properties());
      checked.connect_changed_signal([=] (auto value) {
        if(check_box->get_model()->get_current() != value) {
          check_box->get_model()->set_current(value);
        }
      });
      check_box->get_model()->connect_current_signal([&] (auto is_checked) {
        checked.set(is_checked);
      });
      check_box->get_model()->connect_current_signal(
        profile.make_event_slot<bool>(QString::fromUtf8("CheckedSignal")));
      auto& read_only = get<bool>("read-only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        check_box->set_read_only(is_read_only);
      });
      auto& layout_direction = get<bool>("left-to-right",
        profile.get_properties());
      layout_direction.connect_changed_signal([=] (auto value) {
        if(value) {
          check_box->setLayoutDirection(Qt::LeftToRight);
        } else {
          check_box->setLayoutDirection(Qt::RightToLeft);
        }
      });
      return check_box;
    });
  return profile;
}

UiProfile Spire::make_color_selector_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QColor>("color"));
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
  properties.push_back(make_standard_property<CurrencyId>("currency"));
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
  properties.push_back(
    make_standard_property("current", QString::fromUtf8("1")));
  properties.push_back(
    make_standard_property("minimum", QString::fromUtf8("-100")));
  properties.push_back(
    make_standard_property("maximum", QString::fromUtf8("100")));
  properties.push_back(make_standard_property("decimal_places", 2));
  properties.push_back(make_standard_property("leading_zeros", 0));
  properties.push_back(make_standard_property("trailing_zeros", 2));
  properties.push_back(
    make_standard_property("default_increment", QString::fromUtf8("1")));
  properties.push_back(
    make_standard_property("alt_increment", QString::fromUtf8("5")));
  properties.push_back(
    make_standard_property("ctrl_increment", QString::fromUtf8("10")));
  properties.push_back(
    make_standard_property("shift_increment", QString::fromUtf8("20")));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("buttons_visible", true));
  properties.push_back(make_standard_property("apply_sign_styling", false));
  properties.push_back(make_standard_property("apply_tick_styling", false));
  auto profile = UiProfile(QString::fromUtf8("DecimalBox"), properties,
    [] (auto& profile) {
      auto& width = get<int>("width", profile.get_properties());
      width.set(scale_width(100));
      auto parse_decimal = [] (auto decimal) -> std::optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto model = std::make_shared<LocalScalarValueModel<optional<Decimal>>>();
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
        model->set_increment(pow(Decimal(10), -value));
      });
      auto& default_increment = get<QString>("default_increment",
        profile.get_properties());
      auto& alt_increment = get<QString>("alt_increment",
        profile.get_properties());
      auto& ctrl_increment = get<QString>("ctrl_increment",
        profile.get_properties());
      auto& shift_increment = get<QString>("shift_increment",
        profile.get_properties());
      auto modifiers = QHash<Qt::KeyboardModifier, Decimal>(
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
        if(value == QString::fromUtf8("null")) {
          if(decimal_box->get_model()->get_current()) {
            decimal_box->get_model()->set_current(none);
          }
        } else if(auto decimal = parse_decimal(value)) {
          if(decimal_box->get_model()->get_current() != *decimal) {
            decimal_box->get_model()->set_current(*decimal);
          }
        }
      });
      auto current_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Current"));
      decimal_box->get_model()->connect_current_signal(
        [=, &current] (const optional<Decimal>& value) {
          auto text = [&] {
            if(value) {
              return QString::fromStdString(value->str(
                Decimal::backend_type::cpp_dec_float_digits10,
                std::ios_base::dec));
            }
            return QString::fromUtf8("null");
          }();
          current.set(text);
          current_slot(text);
        });
      auto submit_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Submit"));
      decimal_box->connect_submit_signal(
        [=] (const optional<Decimal>& submission) {
          if(submission) {
            submit_slot(QString::fromStdString(submission->str(
              Decimal::backend_type::cpp_dec_float_digits10,
              std::ios_base::dec)));
          } else {
            submit_slot(QString::fromUtf8("null"));
          }
        });
      auto reject_slot = profile.make_event_slot<QString>(
        QString::fromUtf8("Reject"));
      decimal_box->connect_reject_signal(
        [=] (const optional<Decimal>& value) {
          if(value) {
            reject_slot(QString::fromStdString(value->str(
              Decimal::backend_type::cpp_dec_float_digits10,
              std::ios_base::dec)));
          } else {
            reject_slot(QString::fromUtf8("null"));
          }
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
      auto& apply_sign_styling = get<bool>("apply_sign_styling",
        profile.get_properties());
      apply_sign_styling.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(ReadOnly() && IsPositive()).
            set(TextColor(QColor(0x36BB55)));
          style.get(ReadOnly() && IsNegative()).
            set(TextColor(QColor(0xE63F44)));
        }
        set_style(*decimal_box, std::move(style));
      });
      auto& apply_tick_styling = get<bool>("apply_tick_styling",
        profile.get_properties());
      apply_tick_styling.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(ReadOnly() && Uptick()).
            set(BackgroundColor(
              chain(timeout(QColor(0xEBFFF0), milliseconds(250)),
                linear(QColor(0xEBFFF0), revert, milliseconds(300)))));
          style.get(ReadOnly() && Downtick()).
            set(BackgroundColor(
              chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
                linear(QColor(0xFFF1F1), revert, milliseconds(300)))));
        }
        set_style(*decimal_box, std::move(style));
      });
      return decimal_box;
    });
  return profile;
}

UiProfile Spire::make_duration_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(
    make_standard_property<QString>("minimum", "10:10:10.000"));
  properties.push_back(
    make_standard_property<QString>("maximum", "20:20:20.000"));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile(QString::fromUtf8("DurationBox"), properties,
    [] (auto& profile) {
      auto parse_duration = [] (auto duration) ->
          boost::optional<time_duration> {
        try {
          return boost::posix_time::duration_from_string(
            duration.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto model = std::make_shared<LocalOptionalDurationModel>();
      auto duration_box = new DurationBox(model);
      apply_widget_properties(duration_box, profile.get_properties());
      auto& minimum = get<QString>("minimum", profile.get_properties());
      minimum.connect_changed_signal([=] (auto value) {
        if(auto minimum_value = parse_duration(value)) {
          model->set_minimum(minimum_value);
        }
      });
      auto& maximum = get<QString>("maximum", profile.get_properties());
      maximum.connect_changed_signal([=] (auto value) {
        if(auto maximum_value = parse_duration(value)) {
          model->set_maximum(maximum_value);
        }
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (auto value) {
        if(auto current_value = parse_duration(value)) {
          if(duration_box->get_model()->get_current() != *current_value) {
            duration_box->get_model()->set_current(*current_value);
          }
        }
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        duration_box->set_read_only(is_read_only);
      });
      duration_box->get_model()->connect_current_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Current")));
      duration_box->connect_submit_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Submit")));
      duration_box->connect_reject_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Reject")));
      return duration_box;
    });
  return profile;
}

UiProfile Spire::make_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back( make_standard_property<QString>("title",
    QString::fromUtf8("Filter Quantity")));
  auto profile = UiProfile(QString::fromUtf8("FilterPanel"), properties,
    [] (auto& profile) {
      auto& title = get<QString>("title", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      button->connect_clicked_signal([&, button] {
        auto component = new QWidget();
        component->setObjectName("component");
        component->setStyleSheet("#component {background-color: #F5F5F5;}");
        auto component_layout = new QGridLayout(component);
        component_layout->setSpacing(0);
        component_layout->setContentsMargins({});
        auto min_box = new TextBox(QString::fromUtf8("Min"));
        min_box->set_read_only(true);
        min_box->setFixedSize(scale(40, 30));
        component_layout->addWidget(min_box, 0, 0);
        auto min_text = new TextBox();
        min_text->setFixedSize(scale(120, 26));
        component_layout->addWidget(min_text, 0, 1);
        auto max_box = new TextBox(QString::fromUtf8("Max"));
        max_box->set_read_only(true);
        max_box->setFixedSize(scale(40, 30));
        component_layout->addWidget(max_box, 1, 0);
        auto max_text = new TextBox();
        max_text->setFixedSize(scale(120, 26));
        component_layout->addWidget(max_text, 1, 1);
        auto panel = new FilterPanel(title.get(), component, button);
        panel->window()->setAttribute(Qt::WA_DeleteOnClose);
        panel->connect_reset_signal(profile.make_event_slot(
          QString::fromUtf8("ResetSignal")));
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_flat_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString::fromUtf8("Click me!")));
  auto profile = UiProfile(QString::fromUtf8("LabelButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_label_button(label.get());
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("IconButton"), properties,
    [] (auto& profile) {
      auto button =
        make_icon_button(imageFromSvg(":/Icons/demo.svg", scale(26, 26)));
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_integer_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<int>(properties);
  auto profile = UiProfile(QString::fromUtf8("IntegerBox"), properties,
    setup_decimal_box_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_key_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("key", "f1"));
  auto profile = UiProfile(QString::fromUtf8("KeyTag"), properties,
    [] (auto& profile) {
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
        key_tag->get_model()->set_current(key);
        key_tag->adjustSize();
      });
      return key_tag;
    });
  return profile;
}

UiProfile Spire::make_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("ListItem"), properties,
    [] (auto& profile) {
      auto& width = get<int>("width", profile.get_properties());
      width.set(scale_width(100));
      auto box = new TextBox("Test Component");
      box->setAttribute(Qt::WA_TranslucentBackground);
      box->set_read_only(true);
      box->setDisabled(true);
      auto style = get_style(*box);
      style.get(Disabled()).set(TextColor(QColor::fromRgb(0, 0, 0)));
      set_style(*box, std::move(style));
      auto item = new ListItem(box);
      apply_widget_properties(item, profile.get_properties());
      item->connect_current_signal(
        profile.make_event_slot(QString::fromUtf8("Current")));
      item->connect_submit_signal(
        profile.make_event_slot(QString::fromUtf8("Submit")));
      return item;
    });
  return profile;
}

UiProfile Spire::make_list_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto navigation_property = define_enum<ListView::EdgeNavigation>(
    {{"CONTAIN", ListView::EdgeNavigation::CONTAIN},
     {"WRAP", ListView::EdgeNavigation::WRAP}});
  properties.push_back(
    make_standard_enum_property("edge_navigation", navigation_property));
  auto direction_property = define_enum<Qt::Orientation>(
    {{"Vertical", Qt::Vertical}, {"Horizontal", Qt::Horizontal}});
  properties.push_back(
    make_standard_enum_property("direction", direction_property));
  auto overflow_property = define_enum<ListView::Overflow>(
    {{"NONE", ListView::Overflow::NONE}, {"WRAP", ListView::Overflow::WRAP}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto selection_mode_property = define_enum<ListView::SelectionMode>(
    {{"NONE", ListView::SelectionMode::NONE},
     {"SINGLE", ListView::SelectionMode::SINGLE},
     {"MULTIPLE", ListView::SelectionMode::MULTIPLE}});
  properties.push_back(
    make_standard_enum_property("selection_mode", selection_mode_property));
  auto profile = UiProfile(QString::fromUtf8("ListView"), properties,
    [=] (auto& profile) {
      auto list_model = std::make_shared<ArrayListModel>();
      list_model->push(QString::fromUtf8("AB.NSYE"));
      list_model->push(QString::fromUtf8("ABU.V.CDNX"));
      list_model->push(QString::fromUtf8("CAN"));
      list_model->push(QString::fromUtf8("MSFT.NSDQ"));
      list_model->push(QString::fromUtf8("ABY.AX.ASX"));
      list_model->push(QString::fromUtf8("ABX.NSDQ"));
      list_model->push(QString::fromUtf8("ABX.TO.TSX"));
      list_model->push(QString::fromUtf8("XIU.TSX"));
      list_model->push(QString::fromUtf8("XYZ.TSX"));
      auto current_model = std::make_shared<ListView::LocalCurrentModel>();
      auto list_view = new ListView(current_model, list_model,
        [] (auto model, auto index) {
          auto text_box = new TextBox(model->get<QString>(index));
          text_box->set_read_only(true);
          text_box->setDisabled(true);
          auto style = get_style(*text_box);
          style.get(Disabled()).set(TextColor(QColor::fromRgb(0, 0, 0)));
          set_style(*text_box, std::move(style));
          auto item_widget = new ListItem(text_box);
          item_widget->setFixedHeight(
            scale_height(QRandomGenerator::global()->bounded(30, 70)));
          item_widget->setMinimumWidth(item_widget->sizeHint().width());
          item_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
          return item_widget;
        });
      apply_widget_properties(list_view, profile.get_properties());
      auto& direction = get<Qt::Orientation>("direction",
        profile.get_properties());
      auto& overflow = get<ListView::Overflow>("overflow",
        profile.get_properties());
      direction.connect_changed_signal([=, &overflow] (auto value) {
        if(overflow.get() == ListView::Overflow::WRAP) {
          if(value == Qt::Vertical) {
            list_view->setMaximumWidth(QWIDGETSIZE_MAX);
            list_view->setMinimumWidth(0);
            list_view->setFixedHeight(scale_height(170));
          } else {
            list_view->setMaximumHeight(QWIDGETSIZE_MAX);
            list_view->setMinimumHeight(0);
            list_view->setFixedWidth(scale_width(170));
          }
        } else {
          list_view->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
          list_view->setMinimumSize(0, 0);
        }
        list_view->set_direction(value);
      });
      overflow.connect_changed_signal([=, &direction] (auto value) {
        if(value == ListView::Overflow::WRAP) {
          if(direction.get() == Qt::Vertical) {
            list_view->setMaximumWidth(QWIDGETSIZE_MAX);
            list_view->setMinimumWidth(0);
            list_view->setFixedHeight(scale_height(170));
          } else {
            list_view->setMaximumHeight(QWIDGETSIZE_MAX);
            list_view->setMinimumHeight(0);
            list_view->setFixedWidth(scale_width(170));
          }
        } else {
          list_view->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
          list_view->setMinimumSize(0, 0);
        }
        list_view->set_overflow(value);
      });
      auto& navigation = get<ListView::EdgeNavigation>("edge_navigation",
        profile.get_properties());
      navigation.set(list_view->get_edge_navigation());
      navigation.connect_changed_signal([=] (auto value) {
        list_view->set_edge_navigation(value);
      });
      auto& selection_mode = get<ListView::SelectionMode>("selection_mode",
        profile.get_properties());
      selection_mode.set(list_view->get_selection_mode());
      selection_mode.connect_changed_signal([=] (auto value) {
        list_view->set_selection_mode(value);
      });
      current_model->connect_current_signal(
        profile.make_event_slot<optional<QString>>(
        QString::fromUtf8("Current")));
      list_view->connect_submit_signal(
        profile.make_event_slot<optional<QString>>(
        QString::fromUtf8("Submit")));
      list_view->connect_delete_signal(
        profile.make_event_slot<optional<QString>>(
        QString::fromUtf8("Delete")));
      return list_view;
    });
  return profile;
}

UiProfile Spire::make_money_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<Money>(properties, Money::CENT);
  auto profile = UiProfile(QString::fromUtf8("MoneyBox"), properties,
    setup_decimal_box_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_overlay_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("close_on_blur", true));
  auto positioning_property = define_enum<OverlayPanel::Positioning>(
    {{"NONE", OverlayPanel::Positioning::NONE},
     {"PARENT", OverlayPanel::Positioning::PARENT}});
  properties.push_back(
    make_standard_enum_property("positioning", positioning_property));
  auto profile = UiProfile(QString::fromUtf8("OverlayPanel"), properties,
    [=] (auto& profile) {
      auto& close_on_blur =
        get<bool>("close_on_blur", profile.get_properties());
      auto& positioning = get<OverlayPanel::Positioning>(
        "positioning", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      auto close_on_blur_connection = std::make_shared<scoped_connection>();
      auto positioning_connection = std::make_shared<scoped_connection>();
      auto panel = QPointer<OverlayPanel>();
      button->connect_clicked_signal(
        [=, &profile, &close_on_blur, &positioning] () mutable {
          if(panel && !close_on_blur.get()) {
            return;
          }
          auto body = new QWidget();
          auto container_layout = new QVBoxLayout(body);
          container_layout->setSpacing(0);
          container_layout->setContentsMargins(
            scale_width(1), scale_height(1), scale_width(1), scale_height(1));
          auto title_layout = new QHBoxLayout();
          title_layout->setSpacing(scale_width(3));
          auto title_name = new QLabel(QString::fromUtf8("Filter Date"));
          title_layout->addWidget(title_name);
          auto close_button =
            make_icon_button(imageFromSvg(":/Icons/close.svg", scale(26, 26)));
          close_button->setFixedSize(scale(26, 26));
          close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
          close_button->connect_clicked_signal([=] {
            close_button->window()->close();
          });
          title_layout->addWidget(close_button);
          container_layout->addLayout(title_layout);
          container_layout->addSpacing(scale_height(3));
          auto content_layout = new QGridLayout();
          content_layout->setSpacing(scale_width(5));
          content_layout->setContentsMargins(
            {scale_width(4), scale_height(4), scale_width(4), scale_height(4)});
          content_layout->addWidget(new QLabel(QString::fromUtf8("Start Date:")),
            0, 0);
          auto text_box1 = new TextBox();
          text_box1->setFixedSize(scale(120, 26));
          content_layout->addWidget(text_box1, 0, 1);
          content_layout->addWidget(new QLabel(QString::fromUtf8("End Date:")),
            1, 0);
          auto text_box2 = new TextBox();
          text_box2->setFixedSize(scale(120, 26));
          content_layout->addWidget(text_box2, 1, 1);
          content_layout->addWidget(make_label_button(
            QString::fromUtf8("Reset")), 2, 1);
          container_layout->addLayout(content_layout);
          panel = new OverlayPanel(body, button);
          panel->setAttribute(Qt::WA_DeleteOnClose);
          panel->set_closed_on_blur(close_on_blur.get());
          panel->set_positioning(positioning.get());
          panel->show();
        });
      return button;
    });
  return profile;
}

UiProfile Spire::make_scroll_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("vertical", true));
  properties.push_back(make_standard_property("start_range", 0));
  properties.push_back(make_standard_property("end_range", 1000));
  properties.push_back(make_standard_property("page_size", 100));
  properties.push_back(make_standard_property("line_size", 10));
  properties.push_back(make_standard_property("position", 0));
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
        scroll_bar->resize(scale(13, 200));
      } else {
        scroll_bar->resize(scale(200, 13));
      }
      auto& start_range = get<int>("start_range", profile.get_properties());
      start_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_start = value;
        scroll_bar->set_range(range);
      });
      auto& end_range = get<int>("end_range", profile.get_properties());
      end_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_end = value;
        scroll_bar->set_range(range);
      });
      auto& page_size = get<int>("page_size", profile.get_properties());
      page_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_page_size(value);
      });
      auto& line_size = get<int>("line_size", profile.get_properties());
      line_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_line_size(value);
      });
      auto& position = get<int>("position", profile.get_properties());
      position.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_position(value);
      });
      scroll_bar->connect_position_signal(profile.make_event_slot<int>(
        QString::fromUtf8("Position")));
      return scroll_bar;
    });
  return profile;
}

UiProfile Spire::make_scroll_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto display_policy_property = define_enum<ScrollBox::DisplayPolicy>(
    {{"NEVER", ScrollBox::DisplayPolicy::NEVER},
     {"ALWAYS", ScrollBox::DisplayPolicy::ALWAYS},
     {"ON_OVERFLOW", ScrollBox::DisplayPolicy::ON_OVERFLOW},
     {"ON_ENGAGE", ScrollBox::DisplayPolicy::ON_ENGAGE}});
  properties.push_back(make_standard_enum_property(
    "horizontal_display_policy", display_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_display_policy", display_policy_property));
  auto profile = UiProfile(QString::fromUtf8("ScrollBox"), properties,
    [] (auto& profile) {
      auto label = new QLabel();
      auto reader = QImageReader(":/Icons/color-picker-display.png");
      auto image = QPixmap::fromImage(reader.read());
      image = image.scaled(QSize(2000, 2000));
      label->setPixmap(std::move(image));
      auto scroll_box = new ScrollBox(label);
      scroll_box->resize(scale(320, 240));
      apply_widget_properties(scroll_box, profile.get_properties());
      auto& horizontal_display_policy = get<ScrollBox::DisplayPolicy>(
        "horizontal_display_policy", profile.get_properties());
      horizontal_display_policy.connect_changed_signal(
        [scroll_box] (auto value) {
          scroll_box->set_horizontal(value);
        });
      auto& vertical_display_policy = get<ScrollBox::DisplayPolicy>(
        "vertical_display_policy", profile.get_properties());
      vertical_display_policy.connect_changed_signal([scroll_box] (auto value) {
        scroll_box->set_vertical(value);
      });
      return scroll_box;
    });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  auto profile = UiProfile(QString::fromUtf8("TextBox"), properties,
    [] (auto& profile) {
      auto& width = get<int>("width", profile.get_properties());
      width.set(scale_width(100));
      auto text_box = new TextBox();
      apply_widget_properties(text_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([text_box] (auto is_read_only) {
        text_box->set_read_only(is_read_only);
        text_box->adjustSize();
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& current) {
        if(text_box->get_model()->get_current() != current) {
          text_box->get_model()->set_current(current);
          text_box->adjustSize();
        }
      });
      auto& placeholder = get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([text_box] (const auto& text) {
        text_box->set_placeholder(text);
      });
      text_box->get_model()->connect_current_signal(
        [&, text_box] (const auto& value) {
          current.set(value);
          text_box->adjustSize();
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

UiProfile Spire::make_time_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile(QString::fromUtf8("TimeBox"), properties,
    [] (auto& profile) {
      auto parse_time = [] (auto time) -> boost::optional<time_duration> {
        try {
          return boost::posix_time::duration_from_string(
            time.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto& current = get<QString>("current", profile.get_properties());
      auto time_box = make_time_box();
      apply_widget_properties(time_box, profile.get_properties());
      current.connect_changed_signal([=] (auto value) {
        if(auto current_value = parse_time(value)) {
          if(time_box->get_model()->get_current() != *current_value) {
            time_box->get_model()->set_current(*current_value);
          }
        }
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        time_box->set_read_only(is_read_only);
      });
      time_box->get_model()->connect_current_signal(
        profile.make_event_slot<optional<time_duration>>(
        QString::fromUtf8("Current")));
      time_box->connect_submit_signal(
        profile.make_event_slot<optional<time_duration>>(
        QString::fromUtf8("Submit")));
      time_box->connect_reject_signal(
        profile.make_event_slot<optional<time_duration>>(
        QString::fromUtf8("Reject")));
      return time_box;
    });
  return profile;
}

UiProfile Spire::make_tooltip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("tooltip-text", QString::fromUtf8("Tooltip Text")));
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
