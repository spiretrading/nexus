#include "Spire/UiViewer/Profiles/ScalarFilterPanelProfiles.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  using Spire::to_text;

  QString to_text(const Decimal& value) {
    return to_string(value);
  }

  template<typename B, typename T = typename ScalarFilterPanel<B>::Type>
  auto setup_scalar_filter_panel_profile(UiProfile& profile) {
    using Panel = ScalarFilterPanel<B>;
    using Type = T;
    using PanelType = typename Panel::Type;
    auto from_panel_type = [] (const PanelType& value) {
      if constexpr(std::is_same_v<Type, QString> &&
          std::is_same_v<PanelType, time_duration>) {
        return to_text(value);
      } else {
        return static_cast<Type>(value);
      }
    };
    auto to_panel_type = [] (const Type& value) {
      if constexpr(std::is_same_v<Type, QString> &&
          std::is_same_v<PanelType, time_duration>) {
        return parse_duration(value);
      } else {
        return value;
      }
    };
    auto& min = get<Type>("min", profile.get_properties());
    auto& max = get<Type>("max", profile.get_properties());
    auto range = std::make_shared<LocalValueModel<typename Panel::Range>>();
    range->set({to_panel_type(min.get()), to_panel_type(max.get())});
    auto panel = new Panel(range);
    apply_widget_properties(panel, profile.get_properties());
    min.connect_changed_signal([=] (auto& value) {
      if(auto min_value = to_panel_type(value);
          range->get().m_min != min_value) {
        range->set({min_value, range->get().m_max});
      }
    });
    max.connect_changed_signal([=] (auto& value) {
      if(auto max_value = to_panel_type(value);
          range->get().m_max != max_value) {
        range->set({range->get().m_min, max_value});
      }
    });
    auto filter_slot = profile.make_event_slot<QString>("CurrentSignal");
    range->connect_update_signal(
      [=, &min, &max] (const typename Panel::Range& value) {
        if(value.m_min) {
          if(auto min_value = from_panel_type(*value.m_min);
              min.get() != min_value) {
            min.set(min_value);
          }
        }
        if(value.m_max) {
          if(auto max_value = from_panel_type(*value.m_max);
              max.get() != max_value) {
            max.set(max_value);
          }
        }
        auto to_string = [&] (const auto& value) {
          if(value) {
            return to_text(*value);
          }
          return QString("null");
        };
        filter_slot(QString("%1, %2").
          arg(to_string(value.m_min)).
          arg(to_string(value.m_max)));
      });
    return panel;
  }

  template<typename T>
  void populate_scalar_filter_panel_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    properties.push_back(make_standard_property("min", T(0)));
    properties.push_back(make_standard_property("max", T(100)));
  }

  template<>
  void populate_scalar_filter_panel_properties<QString>(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    properties.push_back(
      make_standard_property<QString>("min", "10:10:10.000"));
    properties.push_back(
      make_standard_property<QString>("max", "20:20:20.000"));
  }
}

UiProfile Spire::make_decimal_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_scalar_filter_panel_properties<Decimal>(properties);
  auto profile = UiProfile("DecimalFilterPanel", properties,
    setup_scalar_filter_panel_profile<DecimalBox>);
  return profile;
}

UiProfile Spire::make_duration_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_scalar_filter_panel_properties<QString>(properties);
  auto profile = UiProfile("DurationFilterPanel", properties,
    setup_scalar_filter_panel_profile<DurationBox, QString>);
  return profile;
}

UiProfile Spire::make_integer_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_scalar_filter_panel_properties<int>(properties);
  auto profile = UiProfile("IntegerFilterPanel", properties,
    setup_scalar_filter_panel_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_money_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_scalar_filter_panel_properties<Money>(properties);
  auto profile = UiProfile("MoneyFilterPanel", properties,
    setup_scalar_filter_panel_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_percent_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("min", Decimal(0)));
  properties.push_back(make_standard_property("max", Decimal(1)));
  auto profile = UiProfile("PercentFilterPanel", properties,
    setup_scalar_filter_panel_profile<PercentBox>);
  return profile;
}

UiProfile Spire::make_quantity_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_scalar_filter_panel_properties<Quantity>(properties);
  auto profile = UiProfile("QuantityFilterPanel", properties,
    setup_scalar_filter_panel_profile<QuantityBox>);
  return profile;
}
