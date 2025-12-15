#ifndef SPIRE_POPULATECANVASNODECONTEXTMENU_HPP
#define SPIRE_POPULATECANVASNODECONTEXTMENU_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Pointers/Out.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/UI/UI.hpp"

class QMenu;

namespace Spire {

  //! Populates a menu with actions specific to a CanvasNode.
  /*!
    \param view The CanvasNodeModel displaying the CanvasNode.
    \param node The CanvasNode whose actions are to populate the menu.
    \param userProfile The user's profile.
    \param menu The menu to populate.
  */
  void PopulateCanvasNodeContextMenu(Beam::Ref<CanvasNodeModel> view,
    const CanvasNode& node, Beam::Ref<UserProfile> userProfile,
    Beam::Out<QMenu> menu);
}

#endif
