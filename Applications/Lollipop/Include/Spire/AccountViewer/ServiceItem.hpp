#ifndef SPIRE_SERVICEITEM_HPP
#define SPIRE_SERVICEITEM_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class ServiceItem
      \brief Represents a service provider.
   */
  class ServiceItem : public AccountViewItem {
    public:

      //! Constructs a ServiceItem.
      /*!
        \param entry The service provider's DirectoryEntry.
      */
      ServiceItem(const Beam::ServiceLocator::DirectoryEntry& entry);

      virtual ~ServiceItem();

      //! Returns the service provider's DirectoryEntry.
      const Beam::ServiceLocator::DirectoryEntry& GetEntry() const;

      virtual Type GetType() const;

      virtual std::string GetName() const;

      virtual QIcon GetIcon() const;

    private:
      Beam::ServiceLocator::DirectoryEntry m_entry;
  };
}

#endif
