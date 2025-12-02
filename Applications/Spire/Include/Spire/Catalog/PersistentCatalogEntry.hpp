#ifndef SPIRE_PERSISTENTCATALOGENTRY_HPP
#define SPIRE_PERSISTENTCATALOGENTRY_HPP
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {

  /*! \class PersistentCatalogEntry
      \brief CatalogEntry that can be saved or deleted.
   */
  class PersistentCatalogEntry : public CatalogEntry {
    public:
      virtual ~PersistentCatalogEntry();

      //! Saves this CatalogEntry.
      virtual void Save() const = 0;

      //! Deletes this CatalogEntry.
      virtual void Delete() const = 0;

    protected:
      friend struct Beam::DataShuttle;

      //! Constructs a PersistentCatalogEntry.
      /*!
        \param uid The entry's uid.
      */
      PersistentCatalogEntry(const boost::uuids::uuid& uid);

      //! Constructs a PersistentCatalogEntry.
      PersistentCatalogEntry();

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void PersistentCatalogEntry::shuttle(S& shuttle, unsigned int version) {
    CatalogEntry::shuttle(shuttle, version);
  }
}

#endif
