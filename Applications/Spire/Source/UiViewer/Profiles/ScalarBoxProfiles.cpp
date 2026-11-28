#include "Spire/UiViewer/Profiles/ScalarBoxProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/PercentBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  template<typename T>
  struct DecimalBoxProfileProperties {
    using Type = T;

    Type m_current;
    Type m_minimum;
    Type m_maximum;
    Type m_default_increment;
    Type m_alt_increment;
    Type m_ctrl_increment;
    Type m_shift_increment;

    explicit DecimalBoxProfileProperties(Type default_increment)
      : m_current(Type(1)),
        m_minimum(Type(-100)),
        m_maximum(Type(100)),
        m_default_increment(std::move(default_increment)),
        m_alt_increment(5 * m_default_increment),
        m_ctrl_increment(10 * m_default_increment),
        m_shift_increment(20 * m_default_increment) {}
  };

  template<typename B>
  auto create_decimal_box_profile(UiProfile& profile) {
    using Type =
      std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar;
    auto model = std::make_shared<LocalScalarValueModel<optional<Type>>>();
    auto& minimum = get<Type>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      model->set_minimum(value);
    });
    auto& maximum = get<Type>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      model->set_maximum(value);
    });
    auto& min_max_enabled =
      get<bool>("min_max_enabled", profile.get_properties());
    min_max_enabled.connect_changed_signal(
      [=, &minimum, &maximum] (auto value) {
        if(value) {
          model->set_minimum(minimum.get());
          model->set_maximum(maximum.get());
        } else {
          model->set_minimum(none);
          model->set_maximum(none);
        }
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
    box->setFixedWidth(scale_width(100));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(box->get_current()->get() != value) {
        box->get_current()->set(value);
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    auto& buttons_visible =
      get<bool>("buttons_visible", profile.get_properties());
    read_only.connect_changed_signal([=, &buttons_visible] (auto value) {
      box->set_read_only(value);
      if(value) {
        buttons_visible.set(false);
      } else {
        buttons_visible.set(true);
      }
    });
    buttons_visible.connect_changed_signal([=] (auto value) {
      update_style(*box, [&] (auto& style) {
        if(value) {
          style.get(Any() > (DownButton() || UpButton())).
            set(Visibility::VISIBLE);
        } else {
          style.get(Any() > (DownButton() || UpButton())).
            set(Visibility::NONE);
        }
      });
    });
    return box;
  }

  template<typename B>
  auto setup_decimal_box_profile(UiProfile& profile) {
    using Type =
      std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar;
    auto box = create_decimal_box_profile<B>(profile);
    box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<Type>>("Current"));
    box->connect_submit_signal(
      profile.make_event_slot<optional<Type>>("Submit"));
    box->connect_reject_signal(
      profile.make_event_slot<optional<Type>>("Reject"));
    return box;
  }

  template<typename B>
  auto setup_decimal_box_with_decimal_profile(UiProfile& profile) {
    auto box = create_decimal_box_profile<B>(profile);
    auto& decimal_places =
      get<int>("decimal_places", profile.get_properties());
    decimal_places.connect_changed_signal([=] (auto value) {
      std::dynamic_pointer_cast<LocalScalarValueModel<optional<Decimal>>>(
        box->get_current())->set_increment(pow(Decimal(10), -value));
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*box, styles);
    });
    auto& current = get<Decimal>("current", profile.get_properties());
    auto current_slot = profile.make_event_slot<QString>("Current");
    box->get_current()->connect_update_signal(
      [=, &current] (const optional<Decimal>& value) {
        auto text = [&] {
          if(value) {
            return to_string(*value);
          }
          return QString("null");
        }();
        if(value) {
          current.set(*value);
        }
        current_slot(text);
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    box->connect_submit_signal(
      [=] (const optional<Decimal>& submission) {
        if(submission) {
          submit_slot(to_string(*submission));
        } else {
          submit_slot(QString("null"));
        }
      });
    auto reject_slot = profile.make_event_slot<QString>("Reject");
    box->connect_reject_signal(
      [=] (const optional<Decimal>& value) {
        if(value) {
          submit_slot(to_string(*value));
        } else {
          reject_slot(QString("null"));
        }
      });
    return box;
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const DecimalBoxProfileProperties<T>& box_properties) {
    using Type = T;
    properties.push_back(
      make_standard_property("current", box_properties.m_current));
    properties.push_back(
      make_standard_property("minimum", box_properties.m_minimum));
    properties.push_back(
      make_standard_property("maximum", box_properties.m_maximum));
    properties.push_back(make_standard_property(
      "default_increment", box_properties.m_default_increment));
    properties.push_back(make_standard_property(
      "alt_increment", box_properties.m_alt_increment));
    properties.push_back(make_standard_property(
      "ctrl_increment", box_properties.m_ctrl_increment));
    properties.push_back(make_standard_property(
      "shift_increment", box_properties.m_shift_increment));
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    properties.push_back(make_standard_property("buttons_visible", true));
    properties.push_back(make_standard_property("min_max_enabled", true));
  }

  void populate_decimal_box_with_decimal_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const DecimalBoxProfileProperties<Decimal>& box_properties) {
    populate_decimal_box_properties<Decimal>(properties, box_properties);
    properties.push_back(make_standard_property("decimal_places", 4));
    auto default_style = R"(
      any {
        leading_zeros: 0;
        trailing_zeros: 0;
        text_align: left;
      }
      :read_only and :is_positive {
        text_color: 0x36BB55;
      }
      :read_only and :is_negative {
        text_color: 0xE63F44;
      }
      :read_only and :uptick {
        background_color: chain(timeout(0xEBFFF0, 250ms),
         linear(0xEBFFF0, revert, 300ms));
      }
      :read_only and :downtick {
        background_color: chain(timeout(0xFFF1F1, 250ms),
         linear(0xFFF1F1, revert, 300ms));
      }
    )";
    properties.push_back(make_style_property("style_sheet",
      std::move(default_style)));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    populate_decimal_box_properties<T>(properties,
      DecimalBoxProfileProperties(1));
  }
}

UiProfile Spire::make_decimal_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_with_decimal_properties(
    properties, DecimalBoxProfileProperties(Decimal(1)));
  auto profile = UiProfile("DecimalBox",
    properties, setup_decimal_box_with_decimal_profile<DecimalBox>);
  return profile;
}

UiProfile Spire::make_integer_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<int>(properties);
  auto profile =
    UiProfile("IntegerBox", properties, setup_decimal_box_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_money_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<Money>(
    properties, DecimalBoxProfileProperties(Money::ONE));
  auto profile =
    UiProfile("MoneyBox", properties, setup_decimal_box_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_percent_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_with_decimal_properties(
    properties, DecimalBoxProfileProperties(Decimal(1)));
  auto profile = UiProfile("PercentBox",
    properties, setup_decimal_box_with_decimal_profile<PercentBox>);
  return profile;
}

UiProfile Spire::make_quantity_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto box_properties = DecimalBoxProfileProperties(Quantity(1));
  box_properties.m_minimum = Quantity(0);
  populate_decimal_box_properties<Quantity>(properties, box_properties);
  auto profile = UiProfile(
    "QuantityBox", properties, setup_decimal_box_profile<QuantityBox>);
  return profile;
}
