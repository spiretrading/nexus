#ifndef SPIRE_ACCOUNTVIEWITEM_HPP
#define SPIRE_ACCOUNTVIEWITEM_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Collections/Enum.hpp>
#include <boost/noncopyable.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

class QIcon;

namespace Spire {
namespace Details {
  BEAM_ENUM(AccountViewItemTypeDefinition,

    //! AdministratorItem
    ADMINISTRATOR,

    //! DirectoryItem
    DIRECTORY,

    //! ManagerItem
    MANAGER,

    //! RootItem
    ROOT,

    //! ServiceItem
    SERVICE,

    //! TraderItem
    TRADER);
}

  /*! \class AccountViewItem
      \brief Base class for an item represented in the AccountViewModel.
   */
  class AccountViewItem : private boost::noncopyable {
    public:

      //! Lists the types of AccountViewItems.
      using Type = Details::AccountViewItemTypeDefinition;

      virtual ~AccountViewItem();

      //! Returns the type of item represented.
      virtual Type GetType() const = 0;

      //! Returns the display name.
      virtual std::string GetName() const = 0;

      //! Returns the icon.
      virtual QIcon GetIcon() const = 0;

      //! Loads this item's children.
      /*!
        \param userProfile The user's profile.
      */
      virtual std::vector<std::unique_ptr<AccountViewItem>> LoadChildren(
        const UserProfile& userProfile) const;
  };
}

#endif
