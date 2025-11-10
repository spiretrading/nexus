#ifndef SPIRE_ADMINISTRATORITEM_HPP
#define SPIRE_ADMINISTRATORITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class AdministratorItem
      \brief Represents an administrator.
   */
  class AdministratorItem : public AccountViewItem {
    public:

      //! Constructs an AdministratorItem.
      /*!
        \param entry The administrator's DirectoryEntry.
      */
      AdministratorItem(const Beam::DirectoryEntry& entry);

      virtual ~AdministratorItem();

      //! Returns the administrator's DirectoryEntry.
      const Beam::DirectoryEntry& GetEntry() const;

      virtual Type GetType() const;

      virtual std::string GetName() const;

      virtual QIcon GetIcon() const;

    private:
      Beam::DirectoryEntry m_entry;
  };
}

#endif
