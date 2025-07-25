#include "Spire/Playback/ReplayAttachMenuButton.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  bool assigned_targets_comparator(const SelectableTarget& left,
      const SelectableTarget& right) {
    return to_text(*left.m_target.m_security) <
      to_text(*right.m_target.m_security);
  }

  bool unassigned_targets_comparator(const SelectableTarget& left,
      const SelectableTarget& right) {
    if(left.m_target.m_window_types.size() == 1 &&
        left.m_target.m_window_types.size() ==
          right.m_target.m_window_types.size()) {
      return to_text(left.m_target.m_window_types[0]) <
        to_text(right.m_target.m_window_types[0]);
    }
    return left.m_target.m_window_types.size() >
      right.m_target.m_window_types.size();
  }

  void add_target_menu_item(ContextMenu& menu,
      std::shared_ptr<SelectableTargetListModel> targets, int index) {
    auto item = new TargetMenuItem(targets->get(index).m_target,
      make_field_value_model(
        make_list_value_model(targets, index), &SelectableTarget::m_selected));
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    menu.add_action("", nullptr, item);
  }
}

MenuButton* Spire::make_replay_attach_menu_button(
    std::shared_ptr<SelectableTargetListModel> targets,
    QWidget* parent) {
  auto button = make_menu_icon_button(
    imageFromSvg(":/Icons/target.svg", scale(26, 26)), QObject::tr("Attach to"));
  button->set_empty_message(QObject::tr("No available windows"));
  auto assigned_targets = std::make_shared<SortedListModel<SelectableTarget>>(
    std::make_shared<FilteredListModel<SelectableTarget>>(targets,
      [] (const ListModel<SelectableTarget>& targets, int index) {
        return !targets.get(index).m_target.m_security.has_value();
      }), assigned_targets_comparator);
  auto unassigned_targets = std::make_shared<SortedListModel<SelectableTarget>>(
    std::make_shared<FilteredListModel<SelectableTarget>>(targets,
      [] (const ListModel<SelectableTarget>& targets, int index) {
        return targets.get(index).m_target.m_security.has_value();
      }), unassigned_targets_comparator);
  auto add_items = [=] {
    auto& menu = button->get_menu();
    for(auto i = 0; i < assigned_targets->get_size(); ++i) {
      add_target_menu_item(menu, assigned_targets, i);
    }
    if(assigned_targets->get_size() > 0 && unassigned_targets->get_size() > 0) {
      menu.add_separator();
    }
    for(auto i = 0; i < unassigned_targets->get_size(); ++i) {
      add_target_menu_item(menu, unassigned_targets, i);
    }
  };
  add_items();
  update_style(button->get_menu(), [] (auto& style) {
    style.get(Any() > is_a<ListView>() > is_a<ListItem>() >
        is_a<TargetMenuItem>() < is_a<ListItem>()).
      set(border_size(0)).
      set(padding(0));
  });
  targets->connect_operation_signal([=] (auto& operation) {
    auto reset_items = [=] {
      button->get_menu().reset();
      add_items();
    };
    visit(operation,
      [&] (const SelectableTargetListModel::AddOperation& operation) {
        reset_items();
      },
      [&] (const SelectableTargetListModel::RemoveOperation& operation) {
        reset_items();
      });
  });
  return button;
}
