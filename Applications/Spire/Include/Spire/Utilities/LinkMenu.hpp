#ifndef SPIRE_LINK_MENU_HPP
#define SPIRE_LINK_MENU_HPP
#include <vector>
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/Ui/ContextMenu.hpp"

namespace Spire {
  class UserProfile;

  /**
   * Adds a ContextMenu sub-menu used to join the PropertyHub that another
   * component belongs to.
   * @param parent The ContextMenu to add the sub-menu to.
   * @param member The member whose PropertyHub is to be selected.
   * @param user_profile The user's profile.
   */
  void add_link_menu(
    ContextMenu& parent, PropertyHubMember& member, UserProfile& user_profile);

  /**
   * Returns the members that a component can join, ordered by name.
   * @param member The member doing the joining.
   * @param user_profile The user's profile.
   */
  std::vector<PropertyHubMember*> find_link_candidates(
    const PropertyHubMember& member, const UserProfile& user_profile);

  /**
   * Joins the PropertyHub that another component belongs to, or a new
   * PropertyHub if the component already belongs to it.
   * @param member The member joining a PropertyHub.
   * @param candidate The member whose PropertyHub is to be joined.
   * @param user_profile The user's profile.
   */
  void toggle_link(PropertyHubMember& member, PropertyHubMember& candidate,
    UserProfile& user_profile);
}

#endif
