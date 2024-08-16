#ifndef SPIRE_CATALOGENTRY_HPP
#define SPIRE_CATALOGENTRY_HPP
#include <memory>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUuid.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <QIcon>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Catalog/Catalog.hpp"

class QMimeData;

namespace Spire {

  /*! \class CatalogEntry
      \brief Contains information about a single entry in a CatalogWindow.
   */
  class CatalogEntry : private boost::noncopyable {
    public:

      //! A CatalogEntry's MIME type.
      static const std::string MIME_TYPE;

      //! Returns a CatalogEntry's CanvasNode meta-data key.
      static const std::string& GetCatalogEntryKey();

      //! Returns a the meta-data key used to identify the CanvasNode's source.
      static const std::string& GetCatalogEntrySourceKey();

      //! Encodes a CatalogEntry as MIME data.
      /*!
        \param entries The list of CatalogEntries to encode.
        \return The MIME representation of the <i>entries</i>.
      */
      static QMimeData* EncodeAsMimeData(
        const std::vector<CatalogEntry*>& entries);

      //! Decodes a CatalogEntry from MIME data.
      /*!
        \param data The MIME data to decode.
        \param settings The settings where the CatalogEntry comes from.
        \return The list of CatalogEntries represented by the MIME <i>data</i>.
      */
      static std::vector<CatalogEntry*> DecodeFromMimeData(
        const QMimeData& data, const CatalogSettings& settings);

      //! Finds the UUID of the CatalogEntry representing a CanvasNode.
      /*!
        \param node The CanvasNode to find the UUID in.
        \return the UUID of the CatalogEntry representing the <i>node</i>.
      */
      static boost::optional<boost::uuids::uuid> FindUuid(
        const CanvasNode& node);

      virtual ~CatalogEntry() = default;

      //! Returns the name of this entry.
      virtual std::string GetName() const = 0;

      //! Returns a new CatalogEntry that renames this one.
      virtual std::unique_ptr<CatalogEntry> SetName(
        const std::string& name) const = 0;

      //! Returns this entry's unique identifier.
      const boost::uuids::uuid& GetUid() const;

      //! Returns the path to the icon.
      virtual const std::string& GetIconPath() const = 0;

      //! Returns a new CatalogEntry that changes the path to the icon.
      virtual std::unique_ptr<CatalogEntry> SetIconPath(
        const std::string& iconPath) const = 0;

      //! Returns the icon.
      virtual QIcon GetIcon() const = 0;

      //! Returns the description of this entry.
      virtual std::string GetDescription() const = 0;

      //! Sets the description.
      /*!
        \param description The new description for this entry.
      */
      virtual void SetDescription(const std::string& description);

      //! Returns the CanvasNode represented by this entry.
      virtual const CanvasNode& GetNode() const = 0;

      //! Returns a new CatalogEntry that changes the CanvasNode represented.
      virtual std::unique_ptr<CatalogEntry> SetNode(
        const CanvasNode& node) const = 0;

      //! Returns <code>true</code> iff this entry is read-only.
      virtual bool IsReadOnly() const = 0;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      //! Constructs a CatalogEntry.
      CatalogEntry() = default;

      //! Constructs a CatalogEntry.
      /*!
        \param uid The entry's uid.
      */
      CatalogEntry(const boost::uuids::uuid& uid);

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      boost::uuids::uuid m_uuid;
  };

  template<typename Shuttler>
  void CatalogEntry::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("uuid", m_uuid);
  }
}

#endif
