#include "Spire/UiViewer/Profiles/ListProfiles.hpp"
#include <QPointer>
#include <QRandomGenerator>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DeletableListItem.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListSelectionModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListViewReorderController.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_deletable_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DeletableListItem", properties,
    [] (auto& profile) {
      auto item = new DeletableListItem(*make_label("XIU.TSX"));
      item->setFixedWidth(scale_width(120));
      apply_widget_properties(item, profile.get_properties());
      item->connect_delete_signal(profile.make_event_slot("DeleteSignal"));
      return item;
  });
  return profile;
}

UiProfile Spire::make_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("current", false));
  properties.push_back(make_standard_property("selected", false));
  auto profile = UiProfile("ListItem", properties, [] (auto& profile) {
    auto item = new ListItem(*make_label("Test Component"));
    item->setFixedWidth(scale_width(100));
    apply_widget_properties(item, profile.get_properties());
    item->connect_submit_signal(profile.make_event_slot("Submit"));
    auto& current = get<bool>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value) {
        match(*item, Current());
      } else {
        unmatch(*item, Current());
      }
    });
    auto& selected = get<bool>("selected", profile.get_properties());
    selected.connect_changed_signal([=] (auto value) {
      item->set_selected(value);
    });
    return item;
  });
  return profile;
}

UiProfile Spire::make_list_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("random_height_seed", 0));
  properties.push_back(make_standard_property("gap", 2));
  properties.push_back(make_standard_property("overflow_gap", 5));
  auto navigation_property = define_enum<EdgeNavigation>(
    {{"CONTAIN", EdgeNavigation::CONTAIN},
     {"WRAP", EdgeNavigation::WRAP}});
  properties.push_back(
    make_standard_enum_property("edge_navigation", navigation_property));
  properties.push_back(
    make_standard_enum_property("direction", get_orientation_property()));
  auto overflow_property = define_enum<Overflow>(
    {{"WRAP", Overflow::WRAP}, {"NONE", Overflow::NONE}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto selection_mode_property = define_enum<ListSelectionModel::Mode>(
    {{"NONE", ListSelectionModel::Mode::NONE},
     {"SINGLE", ListSelectionModel::Mode::SINGLE},
     {"MULTI", ListSelectionModel::Mode::MULTI}});
  properties.push_back(make_standard_enum_property("selection_mode",
    ListSelectionModel::Mode::SINGLE, selection_mode_property));
  auto change_item_property = define_enum<int>({{"Delete", 0}, {"Add", 1}});
  properties.push_back(
    make_standard_enum_property("change_item", change_item_property));
  properties.push_back(make_standard_property("change_item_index", -1));
  properties.push_back(make_standard_property("current_item", -1));
  properties.push_back(make_standard_property("disable_item", -1));
  properties.push_back(make_standard_property("enable_item", -1));
  properties.push_back(make_standard_property("auto_set_current_null", false));
  properties.push_back(make_standard_property("delete_submission", false));
  properties.push_back(make_standard_property("reorderable", true));
  auto profile = UiProfile("ListView", properties, [=] (auto& profile) {
    auto& random_height_seed =
      get<int>("random_height_seed", profile.get_properties());
    auto& direction =
      get<Qt::Orientation>("direction", profile.get_properties());
    auto& change_item = get<int>("change_item", profile.get_properties());
    auto& change_item_index =
      get<int>("change_item_index", profile.get_properties());
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < 66; ++i) {
      if(i == 1) {
        list_model->push("ala");
      } else if(i == 5) {
        list_model->push("alb");
      } else if(i == 6) {
        list_model->push("abc");
      } else if(i == 10) {
        list_model->push("llama");
      } else if(i == 11) {
        list_model->push("llamb");
      } else if(i == 12) {
        list_model->push("lllama");
      } else if(i == 20) {
        list_model->push("llbma");
      } else if(i == 30) {
        list_model->push("llxy");
      } else {
        list_model->push(QString("Item%1").arg(i));
      }
    }
    change_item_index.connect_changed_signal(
      [=, &change_item, index = 0] (auto value) mutable {
        if(value < 0 || value >= list_model->get_size()) {
          return;
        }
        if(change_item.get() == 0) {
          list_model->remove(value);
        } else {
          list_model->insert(QString("newItem%1").arg(index++), value);
        }
      });
    auto item_builder =
      [&, random_generator = QRandomGenerator(random_height_seed.get())]
          (const std::shared_ptr<ListModel<QString>>& model, auto index)
            mutable {
        auto label = make_label(model->get(index));
        if(random_height_seed.get() == 0) {
          auto random_size = random_generator.bounded(30, 70);
          if(direction.get() == Qt::Vertical) {
            label->setFixedHeight(scale_height(random_size));
          } else {
            label->setFixedWidth(scale_height(random_size));
          }
        }
        update_style(*label, [&] (auto& style) {
          style.get(+Any() << Disabled()).set(TextColor(QColor(0xFF0000)));
        });
        return label;
    };
    auto selection_model = std::make_shared<ListSelectionModel>();
    auto list_view = new ListView(list_model, selection_model, item_builder);
    apply_widget_properties(list_view, profile.get_properties());
    auto& gap = get<int>("gap", profile.get_properties());
    gap.connect_changed_signal([=] (auto value) {
      if(value < 0) {
        return;
      }
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(ListItemGap(scale_width(value)));
      });
    });
    auto& overflow_gap = get<int>("overflow_gap", profile.get_properties());
    overflow_gap.connect_changed_signal([=] (auto value) {
      if(value < 0) {
        return;
      }
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(ListOverflowGap(scale_width(value)));
      });
    });
    direction.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& overflow = get<Overflow>("overflow", profile.get_properties());
    overflow.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& selection_mode =
      get<ListSelectionModel::Mode>("selection_mode", profile.get_properties());
    selection_mode.connect_changed_signal([=] (auto value) {
      selection_model->set_mode(value);
    });
    auto& navigation =
      get<EdgeNavigation>("edge_navigation", profile.get_properties());
    navigation.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& current_item = get<int>("current_item", profile.get_properties());
    current_item.connect_changed_signal([=] (auto index) {
      if(index == -1) {
        list_view->get_current()->set(none);
      } else if(index >= 0 && index < list_model->get_size()) {
        list_view->get_current()->set(index);
      }
    });
    auto& disable_item = get<int>("disable_item", profile.get_properties());
    disable_item.connect_changed_signal([=] (auto value) {
      if(auto item = list_view->get_list_item(value)) {
        item->setDisabled(true);
      }
    });
    auto& enable_item = get<int>("enable_item", profile.get_properties());
    enable_item.connect_changed_signal([=] (auto value) {
      if(auto item = list_view->get_list_item(value)) {
        item->setDisabled(false);
      }
    });
    auto& auto_set_current_null =
      get<bool>("auto_set_current_null", profile.get_properties());
    list_view->get_current()->connect_update_signal(
      [&, list_view] (const auto& current) {
        if(current && auto_set_current_null.get()) {
          QTimer::singleShot(2000, [list_view] {
            list_view->get_current()->set(none);
          });
        }
      });
    list_view->get_current()->connect_update_signal(
      profile.make_event_slot<optional<int>>("Current"));
    list_view->connect_submit_signal(
      profile.make_event_slot<optional<std::any>>("Submit"));
    auto selection_slot = profile.make_event_slot<QString>("Selection");
    list_view->get_selection()->connect_operation_signal([=] (auto& operation) {
      visit(operation,
        [&] (const ListView::SelectionModel::AddOperation& operation) {
          selection_slot(QString("Add:%1").arg(
            list_view->get_selection()->get(operation.m_index)));
        },
        [&] (const ListView::SelectionModel::PreRemoveOperation& operation) {
          selection_slot(QString("Remove:%1").arg(
            list_view->get_selection()->get(operation.m_index)));
        },
        [&] (const ListView::SelectionModel::UpdateOperation& operation) {
          selection_slot(QString("Update:%1 to %2").
            arg(operation.get_previous()).arg(operation.get_value()));
        });
    });
    if(get<bool>("delete_submission", profile.get_properties()).get()) {
      list_view->connect_submit_signal([=] (auto& value) {
        if(auto index = list_view->get_current()->get()) {
          list_model->remove(*index);
        }
      });
    }
    auto& reorderable = get<bool>("reorderable", profile.get_properties());
    auto reorder_controller = QPointer<ListViewReorderController>();
    reorderable.connect_changed_signal([=] (auto value) mutable {
      delete reorder_controller;
      if(value) {
        reorder_controller = new ListViewReorderController(*list_view);
      }
    });
    return list_view;
  });
  return profile;
}

UiProfile Spire::make_scrollable_list_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_enum_property("direction", get_orientation_property()));
  auto overflow_property = define_enum<Overflow>(
    {{"NONE", Overflow::NONE}, {"WRAP", Overflow::WRAP}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto display_policy_property = define_enum<ScrollBox::DisplayPolicy>(
    {{"ON_OVERFLOW", ScrollBox::DisplayPolicy::ON_OVERFLOW},
     {"ON_ENGAGE", ScrollBox::DisplayPolicy::ON_ENGAGE}});
  properties.push_back(make_standard_enum_property(
    "horizontal_display_policy", display_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_display_policy", display_policy_property));
  auto profile = UiProfile("ScrollableListBox", properties, [] (auto& profile) {
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < 15; ++i) {
      list_model->push(QString("Item%1").arg(i));
    }
    auto list_view = new ListView(list_model,
      [] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
        return make_label(model->get(index));
      });
    auto scrollable_list_box = new ScrollableListBox(*list_view);
    apply_widget_properties(scrollable_list_box, profile.get_properties());
    auto& direction =
      get<Qt::Orientation>("direction", profile.get_properties());
    direction.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& overflow = get<Overflow>("overflow", profile.get_properties());
    overflow.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& horizontal_display_policy = get<ScrollBox::DisplayPolicy>(
      "horizontal_display_policy", profile.get_properties());
    horizontal_display_policy.connect_changed_signal([=] (auto value) {
      scrollable_list_box->get_scroll_box().set_horizontal(value);
    });
    auto& vertical_display_policy = get<ScrollBox::DisplayPolicy>(
      "vertical_display_policy", profile.get_properties());
    vertical_display_policy.connect_changed_signal([=] (auto value) {
      scrollable_list_box->get_scroll_box().set_vertical(value);
    });
    return scrollable_list_box;
  });
  return profile;
}
