#include "Spire/Utilities/LinkMenu.hpp"
#include <algorithm>
#include <unordered_set>
#include <QHash>
#include <QPointer>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/WindowHighlight.hpp"

using namespace Spire;

namespace {
  Window* find_ancestor_window(QWidget& widget) {
    for(auto candidate = &widget; candidate;
        candidate = candidate->parentWidget()) {
      if(auto window = dynamic_cast<Window*>(candidate)) {
        return window;
      }
    }
    return nullptr;
  }

  const QImage& find_icon(const QString& path) {
    static auto icons = QHash<QString, QImage>();
    auto i = icons.find(path);
    if(i == icons.end()) {
      i = icons.insert(path, image_from_svg(path, scale(10, 10)));
    }
    return *i;
  }

  std::vector<Window*> find_hub_windows(
      const PropertyHub& hub, const UserProfile& user_profile) {
    auto windows = std::vector<Window*>();
    auto& roster = *user_profile.GetPropertyHubMembers();
    for(auto i = 0; i != roster.get_size(); ++i) {
      auto member = roster.get(i);
      if(member->get_hub()->get().get() != &hub) {
        continue;
      }
      if(auto window = dynamic_cast<Window*>(&member->get_component());
          window && window->isVisible()) {
        windows.push_back(window);
      }
    }
    return windows;
  }

  void install_hover_highlight(ContextMenu& submenu,
      std::shared_ptr<std::vector<PropertyHubMember*>> candidates,
      const UserProfile& user_profile) {
    auto source = find_ancestor_window(submenu);
    if(!source) {
      return;
    }
    auto highlight = std::make_shared<WindowHighlight>(
      std::make_shared<ArrayListModel<Window*>>());
    submenu.connect_current_signal(
      [=, &user_profile, source = QPointer<Window>(source)] (
          const auto& current_index) {
        auto group = std::vector<Window*>();
        if(source && current_index && *current_index >= 0 &&
            *current_index < static_cast<int>(candidates->size())) {
          auto& candidate = *(*candidates)[*current_index];
          group = find_hub_windows(*candidate.get_hub()->get(), user_profile);
          if(!std::ranges::contains(group, source.data())) {
            group.push_back(source.data());
          }
        }
        auto& current = *highlight->get_current();
        current.transact([&] {
          auto missing = std::unordered_set(group.begin(), group.end());
          for(auto i = current.get_size() - 1; i >= 0; --i) {
            if(missing.erase(current.get(i)) == 0) {
              current.remove(i);
            }
          }
          for(auto window : group) {
            if(missing.contains(window)) {
              current.push(window);
            }
          }
        });
      });
  }
}

void Spire::add_link_menu(ContextMenu& parent, PropertyHubMember& member,
    UserProfile& user_profile) {
  auto candidates = std::make_shared<std::vector<PropertyHubMember*>>(
    find_link_candidates(member, user_profile));
  auto submenu = new ContextMenu(static_cast<QWidget&>(parent));
  for(auto i = 0; i != static_cast<int>(candidates->size()); ++i) {
    auto& candidate = *(*candidates)[i];
    auto current = std::make_shared<LocalValueModel<bool>>(
      candidate.get_hub()->get() == member.get_hub()->get());
    auto item = new CheckButtonMenuItem(find_icon(candidate.get_icon_path()),
      candidate.get_name()->get(), current);
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    current->connect_update_signal([=, &member, &user_profile, &parent] (auto) {
      toggle_link(member, *(*candidates)[i], user_profile);
      parent.hide();
    });
    submenu->add_action("", [=] {
      item->get_current()->set(!item->get_current()->get());
    }, item);
  }
  parent.add_menu(QObject::tr("Link To"), *submenu);
  install_hover_highlight(*submenu, std::move(candidates), user_profile);
}

std::vector<PropertyHubMember*> Spire::find_link_candidates(
    const PropertyHubMember& member, const UserProfile& user_profile) {
  auto candidates = std::vector<PropertyHubMember*>();
  auto& roster = *user_profile.GetPropertyHubMembers();
  for(auto i = 0; i != roster.get_size(); ++i) {
    auto candidate = roster.get(i);
    if(candidate != &member && candidate->get_component().isVisible()) {
      candidates.push_back(candidate);
    }
  }
  std::sort(candidates.begin(), candidates.end(),
    [] (const auto& left, const auto& right) {
      return left->get_name()->get() < right->get_name()->get();
    });
  return candidates;
}

void Spire::toggle_link(PropertyHubMember& member, PropertyHubMember& candidate,
    UserProfile& user_profile) {
  if(candidate.get_hub()->get() == member.get_hub()->get()) {
    member.get_hub()->set(user_profile.MakePropertyHub());
  } else {
    member.get_hub()->set(candidate.get_hub()->get());
  }
}
