#ifndef SPIRE_MANAGERITEM_HPP
#define SPIRE_MANAGERITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class ManagerItem
      \brief Represents a TradingGroup manager.
   */
  class ManagerItem : public AccountViewItem {
    public:

      //! Constructs a ManagerItem.
      /*!
        \param entry The manager's DirectoryEntry.
      */
      ManagerItem(const Beam::DirectoryEntry& entry);

      virtual ~ManagerItem();

      //! Returns the manager's DirectoryEntry.
      const Beam::DirectoryEntry& GetEntry() const;

      virtual Type GetType() const;
      virtual std::string GetName() const;
      virtual QIcon GetIcon() const;

    private:
      Beam::DirectoryEntry m_entry;
  };
}

#endif
