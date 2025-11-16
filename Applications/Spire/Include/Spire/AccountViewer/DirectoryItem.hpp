#ifndef SPIRE_DIRECTORYITEM_HPP
#define SPIRE_DIRECTORYITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class DirectoryItem
      \brief Represents a directory.
   */
  class DirectoryItem : public AccountViewItem {
    public:

      /*! \enum DirectoryType
          \brief Enumerates the types of directories represented.
       */
      enum class DirectoryType {

        //! Represents a trading group.
        GROUP,

        //! Represents the traders in a trading group.
        TRADERS,

        //! Represents the managers in a trading group.
        MANAGERS,

        //! Used for any other directories.
        OTHER
      };

      //! Constructs a DirectoryItem.
      /*!
        \param entry The directory's DirectoryEntry.
        \param type The type of directory represented.
      */
      DirectoryItem(const Beam::DirectoryEntry& entry,
        DirectoryType type);

      virtual ~DirectoryItem();

      //! Returns the directory's DirectoryEntry.
      const Beam::DirectoryEntry& GetEntry() const;

      //! Returns the DirectoryType represented.
      DirectoryType GetDirectoryType() const;

      virtual Type GetType() const;
      virtual std::string GetName() const;
      virtual QIcon GetIcon() const;
      virtual std::vector<std::unique_ptr<AccountViewItem>> LoadChildren(
        const UserProfile& userProfile) const;

    private:
      Beam::DirectoryEntry m_entry;
      DirectoryType m_directoryType;
  };
}

#endif
