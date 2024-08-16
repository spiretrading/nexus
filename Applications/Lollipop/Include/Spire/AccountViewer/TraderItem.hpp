#ifndef SPIRE_TRADERITEM_HPP
#define SPIRE_TRADERITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class TraderItem
      \brief Represents a trader.
   */
  class TraderItem : public AccountViewItem {
    public:

      //! Constructs a TraderItem.
      /*!
        \param entry The trader's DirectoryEntry.
      */
      TraderItem(const Beam::ServiceLocator::DirectoryEntry& entry);

      virtual ~TraderItem();

      //! Returns the trader's DirectoryEntry.
      const Beam::ServiceLocator::DirectoryEntry& GetEntry() const;

      virtual Type GetType() const;

      virtual std::string GetName() const;

      virtual QIcon GetIcon() const;

    private:
      Beam::ServiceLocator::DirectoryEntry m_entry;
  };
}

#endif
