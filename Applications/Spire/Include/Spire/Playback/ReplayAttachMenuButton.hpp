#ifndef SPIRE_REPLAY_ATTACH_MENU_BUTTON_HPP
#define SPIRE_REPLAY_ATTACH_MENU_BUTTON_HPP
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/Ui/MenuButton.hpp"

namespace Spire {

  /**
   * Returns a MenuButton that attaches the replay to groups of windows.
   * @param roster The members whose PropertyHubs form the attachment targets.
   * @param attachments A list of PropertyHubs that the replay is attached to.
   * @param parent The parent widget.
   */
  MenuButton* make_replay_attach_menu_button(
    std::shared_ptr<ListModel<PropertyHubMember*>> roster,
    std::shared_ptr<PropertyHubListModel> attachments,
    QWidget* parent = nullptr);
}

#endif
