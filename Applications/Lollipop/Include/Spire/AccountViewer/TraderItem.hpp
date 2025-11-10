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
      TraderItem(const Beam::DirectoryEntry& entry);

      virtual ~TraderItem();

      //! Returns the trader's DirectoryEntry.
      const Beam::DirectoryEntry& GetEntry() const;

      virtual Type GetType() const;

      virtual std::string GetName() const;

      virtual QIcon GetIcon() const;

    private:
      Beam::DirectoryEntry m_entry;
  };
}

#endif
