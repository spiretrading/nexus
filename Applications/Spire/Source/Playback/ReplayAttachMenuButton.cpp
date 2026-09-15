#include "Spire/Playback/ReplayAttachMenuButton.hpp"
#include <algorithm>
#include <vector>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/WindowHighlight.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto get_ticker(const std::shared_ptr<PropertyHub>& hub) {
    return hub->get<Ticker>(PropertyHub::TICKER_PROPERTY);
  }

  class AttachmentModel : public ValueModel<bool> {
    public:
      AttachmentModel(std::shared_ptr<PropertyHubListModel> attachments,
          std::shared_ptr<PropertyHub> hub)
          : m_attachments(std::move(attachments)),
            m_hub(std::move(hub)),
            m_is_attached(find() != -1) {
        m_connection = m_attachments->connect_operation_signal(
          std::bind_front(&AttachmentModel::on_operation, this));
      }

      const Type& get() const override {
        return m_is_attached;
      }

      QValidator::State test(const Type& value) const override {
        return QValidator::State::Acceptable;
      }

      QValidator::State set(const Type& value) override {
        if(value == m_is_attached) {
          return QValidator::State::Acceptable;
        }
        if(value) {
          m_attachments->push(m_hub);
        } else if(auto index = find(); index != -1) {
          m_attachments->remove(index);
        }
        return QValidator::State::Acceptable;
      }

      connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const override {
        return m_update_signal.connect(slot);
      }

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<PropertyHubListModel> m_attachments;
      std::shared_ptr<PropertyHub> m_hub;
      bool m_is_attached;
      scoped_connection m_connection;

      int find() const {
        for(auto i = 0; i < m_attachments->get_size(); ++i) {
          if(m_attachments->get(i) == m_hub) {
            return i;
          }
        }
        return -1;
      }

      void on_operation(const PropertyHubListModel::Operation&) {
        auto is_attached = find() != -1;
        if(is_attached != m_is_attached) {
          m_is_attached = is_attached;
          m_update_signal(m_is_attached);
        }
      }
  };

  class ReplayAttachMenu : public QObject {
    public:
      ReplayAttachMenu(MenuButton& button,
          std::shared_ptr<ListModel<PropertyHubMember*>> roster,
          std::shared_ptr<PropertyHubListModel> attachments)
          : QObject(&button),
            m_button(&button),
            m_roster(std::move(roster)),
            m_attachments(std::move(attachments)),
            m_highlight(std::make_shared<ArrayListModel<Window*>>()),
            m_is_rebuild_pending(false) {
        m_roster_connection = m_roster->connect_operation_signal(
          [=] (const auto&) {
            clear_items();
            schedule_rebuild();
          });
        m_current_connection = m_button->get_menu().connect_current_signal(
          std::bind_front(&ReplayAttachMenu::on_current, this));
        rebuild();
      }

    private:
      struct Target {
        std::shared_ptr<PropertyHub> m_hub;
        std::vector<PropertyHubMember*> m_members;
        Ticker m_ticker;
        QString m_name;
      };
      MenuButton* m_button;
      std::shared_ptr<ListModel<PropertyHubMember*>> m_roster;
      std::shared_ptr<PropertyHubListModel> m_attachments;
      std::vector<scoped_connection> m_connections;
      WindowHighlight m_highlight;
      bool m_is_rebuild_pending;
      std::vector<std::vector<PropertyHubMember*>> m_items;
      scoped_connection m_roster_connection;
      scoped_connection m_current_connection;

      static bool compare_targets(const Target& left, const Target& right) {
        if(static_cast<bool>(left.m_ticker) !=
            static_cast<bool>(right.m_ticker)) {
          return static_cast<bool>(left.m_ticker);
        }
        if(left.m_ticker) {
          auto left_ticker_name = to_text(left.m_ticker);
          auto right_ticker_name = to_text(right.m_ticker);
          if(left_ticker_name != right_ticker_name) {
            return left_ticker_name < right_ticker_name;
          }
        } else if(left.m_members.size() != right.m_members.size()) {
          return left.m_members.size() > right.m_members.size();
        }
        return left.m_name < right.m_name;
      }

      static std::vector<Target> make_targets(
          const ListModel<PropertyHubMember*>& roster) {
        auto targets = std::vector<Target>();
        for(auto i = 0; i < roster.get_size(); ++i) {
          auto member = roster.get(i);
          auto& hub = member->get_hub()->get();
          auto group = std::ranges::find(targets, hub, &Target::m_hub);
          if(group == targets.end()) {
            auto& target = targets.emplace_back();
            target.m_hub = hub;
            target.m_members.push_back(member);
          } else {
            group->m_members.push_back(member);
          }
        }
        for(auto& target : targets) {
          target.m_ticker = get_ticker(target.m_hub)->get();
          if(target.m_members.size() == 1) {
            target.m_name = target.m_members.front()->get_name();
          } else if(target.m_ticker) {
            target.m_name = QObject::tr("%1 (%2)").
              arg(to_text(target.m_ticker)).
              arg(target.m_members.size());
          } else {
            target.m_name =
              QObject::tr("Unassigned (%1)").arg(target.m_members.size());
          }
        }
        return targets;
      }

      void collect_attachments(const std::vector<Target>& targets) {
        for(auto i = m_attachments->get_size() - 1; i >= 0; --i) {
          if(std::ranges::find(targets, m_attachments->get(i),
              &Target::m_hub) == targets.end()) {
            m_attachments->remove(i);
          }
        }
      }

      void observe(const std::vector<Target>& targets) {
        auto on_update = [=] (const auto&) { schedule_rebuild(); };
        for(auto& target : targets) {
          m_connections.push_back(
            get_ticker(target.m_hub)->connect_update_signal(on_update));
          for(auto member : target.m_members) {
            m_connections.push_back(
              member->get_hub()->connect_update_signal(on_update));
          }
        }
      }

      void render(std::vector<Target> targets) {
        std::stable_sort(targets.begin(), targets.end(), compare_targets);
        auto unassigned = std::ranges::find_if(targets,
          [] (auto& target) {
            return !target.m_ticker;
          });
        auto& menu = m_button->get_menu();
        clear_items();
        menu.reset();
        for(auto i = targets.begin(); i != targets.end(); ++i) {
          if(i == unassigned && i != targets.begin()) {
            menu.add_separator();
            m_items.emplace_back();
          }
          menu.add_check_box(i->m_name, std::make_shared<AttachmentModel>(
            m_attachments, i->m_hub));
          m_items.push_back(i->m_members);
        }
      }

      void rebuild() {
        m_connections.clear();
        auto targets = make_targets(*m_roster);
        collect_attachments(targets);
        observe(targets);
        render(std::move(targets));
      }

      void schedule_rebuild() {
        if(m_is_rebuild_pending) {
          return;
        }
        m_is_rebuild_pending = true;
        QTimer::singleShot(0, this, [=] {
          m_is_rebuild_pending = false;
          rebuild();
        });
      }

      void clear_items() {
        m_items.clear();
        clear(*m_highlight.get_current());
      }

      void on_current(const optional<int>& current) {
        auto& highlight = *m_highlight.get_current();
        highlight.transact([&] {
          clear(highlight);
          if(!current || *current < 0 ||
              *current >= static_cast<int>(m_items.size())) {
            return;
          }
          for(auto member : m_items[*current]) {
            if(auto window = dynamic_cast<Window*>(&member->get_component())) {
              highlight.push(window);
            }
          }
        });
      }
  };
}

MenuButton* Spire::make_replay_attach_menu_button(
    std::shared_ptr<ListModel<PropertyHubMember*>> roster,
    std::shared_ptr<PropertyHubListModel> attachments, QWidget* parent) {
  auto button = make_menu_icon_button(
    image_from_svg(":/Icons/target.svg", scale(26, 26)),
    QObject::tr("Attach to"), parent);
  button->set_empty_message(QObject::tr("No available windows"));
  new ReplayAttachMenu(*button, std::move(roster), std::move(attachments));
  return button;
}
