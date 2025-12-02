#include "Spire/Utilities/LinkMenu.hpp"
#include <QApplication>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ContextMenu.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  struct LinkModel : BooleanModel {
    SecurityContext* m_source;
    SecurityContext* m_target;
    LocalBooleanModel m_value;

    LinkModel(SecurityContext& source, SecurityContext& target)
      : m_source(&source),
        m_target(&target),
        m_value(m_source->GetLinkedIdentifier() == m_target->GetIdentifier()) {}

    const Type& get() const override {
      return m_value.get();
    }

    QValidator::State set(const bool& value) {
      if(value == get()) {
        return QValidator::State::Acceptable;
      }
      if(value) {
        m_source->Link(*m_target);
        if(m_target->GetLinkedIdentifier().empty()) {
          m_target->Link(*m_source);
        }
      } else {
        m_source->Unlink();
      }
      return m_value.set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_value.connect_update_signal(slot);
    }
  };
}

void Spire::add_link_menu(ContextMenu& parent, SecurityContext& context) {
  struct Item {
    Security m_security;
    QString m_title;
    std::shared_ptr<BooleanModel> m_is_linked;
  };
  auto link_menu = new ContextMenu(static_cast<QWidget&>(parent));
  auto windows = QApplication::topLevelWidgets();
  auto items = std::vector<Item>();
  for(auto& window : windows) {
    auto window_context = dynamic_cast<SecurityContext*>(window);
    if(window->isVisible() && window_context != nullptr &&
        window_context != &context) {
      auto item = Item(window_context->GetDisplayedSecurity(),
        window->windowTitle(),
        std::make_shared<LinkModel>(context, *window_context));
      items.push_back(std::move(item));
    }
  }
  std::sort(items.begin(), items.end(),
    [&] (const auto& left, const auto& right) {
      return to_text(left.m_security) < to_text(right.m_security);
    });
  for(auto& item : items) {
    link_menu->add_check_box(item.m_title, item.m_is_linked);
  }
  parent.add_menu(QObject::tr("Link to"), *link_menu);
}
