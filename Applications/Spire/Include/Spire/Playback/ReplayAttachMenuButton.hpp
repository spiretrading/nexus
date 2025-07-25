#ifndef SPIRE_REPLAY_ATTACH_MENU_BUTTON_HPP
#define SPIRE_REPLAY_ATTACH_MENU_BUTTON_HPP
#include "Spire/Playback/Playback.hpp"
#include "Spire/Playback/TargetMenuItem.hpp"
#include "Spire/Ui/MenuButton.hpp"

namespace Spire {

  /** Represents a target with a selection flag. */
  struct SelectableTarget {

    /** The replay attachment target*/
    TargetMenuItem::Target m_target;

    /** Whether the target is selected. */
    bool m_selected;
  };

  /** The type of model representing the list of selectable targets. */
  using SelectableTargetListModel = ListModel<SelectableTarget>;

  /**
   * Returns a MenuButton configured as ReplayAttachMenuButton.
   * @param targets The replay attachment targets that user can select.
   * @param parent The parent widget.
   */
  MenuButton* make_replay_attach_menu_button(
    std::shared_ptr<SelectableTargetListModel> targets,
    QWidget* parent = nullptr);
}

#endif
