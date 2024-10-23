#include "Spire/Utilities/LinkMenu.hpp"
#include "Spire/Ui/ContextMenu.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

void Spire::add_link_menu(ContextMenu& parent, SecurityContext& window,
    const MarketDatabase& markets) {
  auto link_menu = new ContextMenu(static_cast<QWidget&>(parent));
  parent.add_menu(QObject::tr("Link to"), *link_menu);
}
