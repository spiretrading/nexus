#include "Spire/UiViewer/Profiles/OrderFieldProfiles.hpp"
#include "Spire/KeyBindings/OrderFieldInfoTip.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"
#include "Spire/Ui/OrderStatusBox.hpp"
#include "Spire/Ui/OrderStatusFilterPanel.hpp"
#include "Spire/Ui/OrderStatusListBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/OrderTypeFilterPanel.hpp"
#include "Spire/Ui/PositionSideBox.hpp"
#include "Spire/Ui/PositionSideFilterPanel.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/SideFilterPanel.hpp"
#include "Spire/Ui/TaskStateBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"
#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T,
    typename ClosedFilterPanel* (*f)(std::shared_ptr<ListModel<T>>, QWidget*),
    typename Resolver>
  auto setup_closed_filter_panel_profile_impl(UiProfile& profile,
      Resolver resolve) {
    auto& properties = profile.get_properties();
    auto checked_properties = std::vector<std::shared_ptr<UiProperty>>();
    auto model = std::make_shared<ArrayListModel<T>>();
    for(auto property : properties) {
      if(property->get_name() != "enabled" && property->get_name() != "width" &&
          property->get_name() != "height") {
        checked_properties.push_back(property);
        model->push(resolve(property->get_name()));
      }
    }
    auto panel = f(model, nullptr);
    apply_widget_properties(panel, properties);
    for(auto i = 0; i < static_cast<int>(checked_properties.size()); ++i) {
      auto& checked =
        get<bool>(checked_properties[i]->get_name(), profile.get_properties());
      checked.connect_changed_signal([=] (const auto& value) {
        if(panel->get_table()->get<bool>(i, 1) != value) {
          panel->get_table()->set(i, 1, value);
        }
      });
    }
    panel->get_table()->connect_operation_signal(
      [=, &profile] (const TableModel::Operation& operation) {
        visit(operation,
          [=, &profile] (const TableModel::UpdateOperation& operation) {
            auto value = panel->get_table()->get<bool>(operation.m_row, 1);
            auto& checked =
              get<bool>(checked_properties[operation.m_row]->get_name(),
              profile.get_properties());
            if(checked.get() != value) {
              checked.set(value);
            }
          });
      });
    auto submit_filter_slot = profile.make_event_slot<QString>("SubmitSignal");
    panel->connect_submit_signal(
      [=] (const std::shared_ptr<AnyListModel>& submission) {
        auto result = QString();
        for(auto i = 0; i < submission->get_size(); ++i) {
          result += to_text(submission->get(i)) + " ";
        }
        submit_filter_slot(result);
      });
    return panel;
  }

  template<typename T,
    typename ClosedFilterPanel* (*f)(std::shared_ptr<ListModel<T>>, QWidget*)>
  auto setup_closed_filter_panel_profile(UiProfile& profile) {
    return setup_closed_filter_panel_profile_impl<T, f>(profile,
      [] (const QString& name) { return *from_text<T>(name); });
  }

  template<typename T,
    typename ClosedFilterPanel* (*f)(std::shared_ptr<ListModel<T>>, QWidget*)>
  auto setup_named_closed_filter_panel_profile(UiProfile& profile,
      const std::vector<std::pair<QString, T>>& values) {
    return setup_closed_filter_panel_profile_impl<T, f>(profile,
      [&] (const QString& name) -> T {
        return std::find_if(values.begin(), values.end(),
          [&] (const auto& value) { return value.first == name; })->second;
      });
  }

  template<typename B, typename B* (*F)(QWidget*), typename... Converters>
  auto setup_enum_box_profile(UiProfile& profile,
      const Converters&... converters) {
    using Type = B::Type;
    auto box = F(nullptr);
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      box->set_read_only(is_read_only);
    });
    box->connect_submit_signal(
      profile.make_event_slot<std::any>("Submit", converters...));
    return box;
  }
}

UiProfile Spire::make_order_field_info_tip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("name", QString("TSXPegType")));
  properties.push_back(make_standard_property("description", QString(
    "Peg to the protected NBBO. Available on undisplayed orders only.")));
  properties.push_back(make_standard_property("value1", QString(
    "C,Contra Midpoint Only Plus. Order is priced at the Protected NBBO "
    "midpoint. Can trade only against other Contra Midpoint Only Plus "
    "orders.")));
  properties.push_back(make_standard_property("value2", QString(
    "D,Contra Midpoint Only Plus, Dark Sweep. Order is priced at the protected "
    "NBBO midpoint. Can trade against all dark orders on entry, and only "
    "against other Contra Midpoint Only Plus orders once resting.")));
  properties.push_back(make_standard_property(
    "value3", QString("M,Subject to the order's optional limit price.")));
  properties.push_back(make_standard_property(
    "value4", QString("Q,Additional value.")));
  properties.push_back(make_standard_property(
    "prereq1", QString("Prerequisite1,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq2", QString("Prerequisite2,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq3", QString("Prerequisite3,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq4", QString("Prerequisite4,A,B,C,D,E")));
  auto profile = UiProfile("OrderFieldInfoTip", properties, [] (auto& profile) {
    auto label = make_label("Hover me!");
    apply_widget_properties(label, profile.get_properties());
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name =
      get<QString>("name", profile.get_properties()).get().toStdString();
    model.m_tag.m_description = get<QString>("description",
      profile.get_properties()).get().toStdString();
    auto parse_value = [] (const auto& text) {
      auto value = OrderFieldInfoTip::Model::Argument();
      auto list = text.split(",");
      if(!list.isEmpty()) {
        value.m_value = list.front().toStdString();
        if(list.size() > 1) {
          value.m_description = list[1].toStdString();
        }
      }
      return value;
    };
    auto parse_tag = [] (const auto& text) {
      auto tag = OrderFieldInfoTip::Model::Tag();
      auto list = text.split(",");
      for(auto& item : list) {
        if(tag.m_name.empty()) {
          tag.m_name = item.toStdString();
          continue;
        }
        tag.m_arguments.push_back({item.toStdString(), ""});
      }
      return tag;
    };
    for(auto i = 1; i < 5; ++i) {
      auto& value =
        get<QString>(QString("value%1").arg(i), profile.get_properties());
      if(auto text = value.get(); !text.isEmpty()) {
        model.m_tag.m_arguments.push_back(parse_value(text));
      }
      auto& prereq =
        get<QString>(QString("prereq%1").arg(i), profile.get_properties());
      if(auto text = prereq.get(); !text.isEmpty()) {
        model.m_prerequisites.push_back(parse_tag(text));
      }
    }
    auto tip = new OrderFieldInfoTip(std::move(model), label);
    return label;
  });
  return profile;
}

UiProfile Spire::make_order_status_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<OrderStatus>(
    {{"Pending New", OrderStatus::PENDING_NEW},
     {"Rejected", OrderStatus::REJECTED},
     {"New", OrderStatus::NEW},
     {"Partially Filled", OrderStatus::PARTIALLY_FILLED},
     {"Expired", OrderStatus::EXPIRED},
     {"Canceled", OrderStatus::CANCELED},
     {"Suspended", OrderStatus::SUSPENDED},
     {"Stopped", OrderStatus::STOPPED},
     {"Filled", OrderStatus::FILLED},
     {"Done For Day", OrderStatus::DONE_FOR_DAY},
     {"Pending Cancel", OrderStatus::PENDING_CANCEL},
     {"Cancel Reject", OrderStatus::CANCEL_REJECT}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("OrderStatusBox", properties,
    std::bind_front(
      setup_enum_box_profile<OrderStatusBox, make_order_status_box>));
  return profile;
}

UiProfile Spire::make_order_status_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("Pending New"));
  properties.push_back(make_standard_property<bool>("New"));
  properties.push_back(make_standard_property<bool>("Partially Filled"));
  properties.push_back(make_standard_property<bool>("Rejected"));
  properties.push_back(make_standard_property<bool>("Expired"));
  properties.push_back(make_standard_property<bool>("Canceled"));
  properties.push_back(make_standard_property<bool>("Suspended"));
  properties.push_back(make_standard_property<bool>("Stopped"));
  properties.push_back(make_standard_property<bool>("Filled"));
  properties.push_back(make_standard_property<bool>("Done For Day"));
  properties.push_back(make_standard_property<bool>("Pending Cancel"));
  properties.push_back(make_standard_property<bool>("Cancel Reject"));
  auto profile = UiProfile("OrderStatusFilterPanel", properties,
    [] (auto& profile) {
      auto statuses = std::array{
        std::pair{OrderStatus::PENDING_NEW, "Pending New"},
        std::pair{OrderStatus::NEW, "New"},
        std::pair{OrderStatus::PARTIALLY_FILLED, "Partially Filled"},
        std::pair{OrderStatus::REJECTED, "Rejected"},
        std::pair{OrderStatus::EXPIRED, "Expired"},
        std::pair{OrderStatus::CANCELED, "Canceled"},
        std::pair{OrderStatus::SUSPENDED, "Suspended"},
        std::pair{OrderStatus::STOPPED, "Stopped"},
        std::pair{OrderStatus::FILLED, "Filled"},
        std::pair{OrderStatus::DONE_FOR_DAY, "Done For Day"},
        std::pair{OrderStatus::PENDING_CANCEL, "Pending Cancel"},
        std::pair{OrderStatus::CANCEL_REJECT, "Cancel Reject"}};
      auto current = std::make_shared<ArrayListModel<OrderStatus>>();
      for(auto& [status, name] : statuses) {
        auto& checked = get<bool>(name, profile.get_properties());
        if(checked.get()) {
          current->push(status);
        }
      }
      auto panel = new OrderStatusFilterPanel(std::move(current));
      apply_widget_properties(panel, profile.get_properties());
      auto submit_slot = profile.make_event_slot<QString>("Submit");
      panel->connect_submit_signal(
        [=] (const auto& submission) {
          auto items = QString();
          for(auto i = 0; i < submission->get_size(); ++i) {
            if(!items.isEmpty()) {
              items += ", ";
            }
            items += to_text(submission->get(i));
          }
          submit_slot(items);
        });
      return panel;
    });
  return profile;
}

UiProfile Spire::make_order_status_list_box_profile() {
  return setup_tag_combo_box_profile("OrderStatusListBox",
    [] { return make_order_status_list_box(); });
}

UiProfile Spire::make_order_type_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<OrderType>(
    {{"Limit", OrderType::LIMIT},
     {"Market", OrderType::MARKET},
     {"Pegged", OrderType::PEGGED},
     {"Stop", OrderType::STOP}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("OrderTypeBox", properties,
    std::bind_front(setup_enum_box_profile<OrderTypeBox, make_order_type_box>));
  return profile;
}

UiProfile Spire::make_order_type_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("Limit"));
  properties.push_back(make_standard_property<bool>("Market"));
  properties.push_back(make_standard_property<bool>("Pegged"));
  properties.push_back(make_standard_property<bool>("Stop"));
  auto profile = UiProfile("OrderTypeFilterPanel", properties,
    std::bind_front(setup_closed_filter_panel_profile<
      OrderType, make_order_type_filter_panel>));
  return profile;
}

UiProfile Spire::make_position_side_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<Side>(
    {{"Long", Side::BID}, {"Short", Side::ASK}, {"Flat", Side::NONE}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("PositionSideBox", properties,
    [] (auto& profile) {
      return setup_enum_box_profile<SideBox, make_position_side_box>(
        profile, [] (const std::any& value) {
          return PositionSide(std::any_cast<Side>(value));
        });
    });
  return profile;
}

UiProfile Spire::make_position_side_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("Long"));
  properties.push_back(make_standard_property<bool>("Short"));
  properties.push_back(make_standard_property<bool>("Flat"));
  auto profile = UiProfile("PositionSideFilterPanel", properties,
    [] (auto& profile) {
      return setup_named_closed_filter_panel_profile<
        Side, make_position_side_filter_panel>(profile, define_enum<Side>(
          {{"Long", Side::BID}, {"Short", Side::ASK}, {"Flat", Side::NONE}}));
    });
  return profile;
}

UiProfile Spire::make_side_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<Side>(
    {{"Bid", Side::BID}, {"Ask", Side::ASK}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("SideBox", properties, std::bind_front(
    setup_enum_box_profile<SideBox, make_side_box>));
  return profile;
}

UiProfile Spire::make_side_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("Bid"));
  properties.push_back(make_standard_property<bool>("Ask"));
  auto profile = UiProfile("SideFilterPanel", properties, std::bind_front(
    setup_closed_filter_panel_profile<Side, make_side_filter_panel>));
  return profile;
}

UiProfile Spire::make_task_state_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<Task::State>(
    {{"Ready", Task::State::READY},
     {"Initializing", Task::State::INITIALIZING},
     {"Active", Task::State::ACTIVE},
     {"Pending Cancel", Task::State::PENDING_CANCEL},
     {"Canceled", Task::State::CANCELED},
     {"Complete", Task::State::COMPLETE},
     {"Failed", Task::State::FAILED}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("TaskStateBox", properties,
    std::bind_front(setup_enum_box_profile<TaskStateBox, make_task_state_box>));
  return profile;
}

UiProfile Spire::make_time_in_force_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<TimeInForce>(
    {{"Day", TimeInForce(TimeInForce::Type::DAY)},
     {"GTC", TimeInForce(TimeInForce::Type::GTC)},
     {"OPG", TimeInForce(TimeInForce::Type::OPG)},
     {"IOC", TimeInForce(TimeInForce::Type::IOC)},
     {"FOK", TimeInForce(TimeInForce::Type::FOK)},
     {"GTX", TimeInForce(TimeInForce::Type::GTX)},
     {"GTD", TimeInForce(TimeInForce::Type::GTD)},
     {"MOC", TimeInForce(TimeInForce::Type::MOC)}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("TimeInForceBox", properties, std::bind_front(
    setup_enum_box_profile<TimeInForceBox, make_time_in_force_box>));
  return profile;
}

UiProfile Spire::make_time_in_force_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("DAY"));
  properties.push_back(make_standard_property<bool>("GTC"));
  properties.push_back(make_standard_property<bool>("OPG"));
  properties.push_back(make_standard_property<bool>("IOC"));
  properties.push_back(make_standard_property<bool>("FOK"));
  properties.push_back(make_standard_property<bool>("GTX"));
  properties.push_back(make_standard_property<bool>("GTD"));
  properties.push_back(make_standard_property<bool>("MOC"));
  auto profile = UiProfile("TimeInForceFilterPanel", properties,
    std::bind_front(setup_closed_filter_panel_profile<
      TimeInForce, make_time_in_force_filter_panel>));
  return profile;
}
