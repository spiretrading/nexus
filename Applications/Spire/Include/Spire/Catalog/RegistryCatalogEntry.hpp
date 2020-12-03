#ifndef SPIRE_REGISTRYCATALOGENTRY_HPP
#define SPIRE_REGISTRYCATALOGENTRY_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Catalog/PersistentCatalogEntry.hpp"

namespace Spire {

  /*! \class RegistryCatalogEntry
      \brief CatalogEntry stored in Spire's registry.
   */
  class RegistryCatalogEntry : public PersistentCatalogEntry {
    public:

      //! Returns the meta-data value used to indicate the CanvasNode's source.
      static const std::string& GetRegistrySourceValue();

      //! Constructs an uninitialized RegistryCatalogEntry.
      /*!
        \param isReadOnly Whether this RegistryCatalogEntry can be edited.
        \param path The path in the registry to save this CatalogEntry to.
        \param registryClient The RegistryClient used to save this CatalogEntry.
      */
      RegistryCatalogEntry(bool isReadOnly, const std::string& path,
        Beam::RegistryService::RegistryClientBox registryClient);

      //! Constructs a RegistryCatalogEntry.
      /*!
        \param name The name of the entry.
        \param iconPath The path to the entry's icon.
        \param description The description of the entry.
        \param node The CanvasNode this entry represents.
        \param path The path in the registry to save this CatalogEntry to.
        \param registryClient The RegistryClient used to save this CatalogEntry.
      */
      RegistryCatalogEntry(const std::string& name, const std::string& iconPath,
        const std::string& description, const CanvasNode& node,
        const std::string& path,
        Beam::RegistryService::RegistryClientBox registryClient);

      //! Constructs a RegistryCatalogEntry.
      /*!
        \param name The name of the entry.
        \param iconPath The path to the entry's icon.
        \param description The description of the entry.
        \param node The CanvasNode this entry represents.
        \param uid The CatalogEntry's unique id.
        \param path The path in the registry to save this CatalogEntry to.
        \param registryClient The RegistryClient used to save this CatalogEntry.
      */
      RegistryCatalogEntry(const std::string& name, const std::string& iconPath,
        const std::string& description, const CanvasNode& node,
        const boost::uuids::uuid& uid, const std::string& path,
        Beam::RegistryService::RegistryClientBox registryClient);

      virtual void Save() const;

      virtual void Delete() const;

      virtual std::string GetName() const;

      virtual std::unique_ptr<CatalogEntry> SetName(
        const std::string& name) const;

      virtual const std::string& GetIconPath() const;

      virtual std::unique_ptr<CatalogEntry> SetIconPath(
        const std::string& iconPath) const;

      virtual QIcon GetIcon() const;

      virtual std::string GetDescription() const;

      virtual const CanvasNode& GetNode() const;

      virtual std::unique_ptr<CatalogEntry> SetNode(
        const CanvasNode& node) const;

      virtual bool IsReadOnly() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      bool m_isReadOnly;
      std::string m_name;
      std::string m_iconPath;
      QIcon m_icon;
      std::string m_description;
      std::unique_ptr<CanvasNode> m_node;
      std::string m_path;
      mutable Beam::RegistryService::RegistryClientBox m_registryClient;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
      void Validate();
  };

  template<typename Shuttler>
  void RegistryCatalogEntry::Shuttle(Shuttler& shuttle, unsigned int version) {
    PersistentCatalogEntry::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("icon_path", m_iconPath);
    shuttle.Shuttle("description", m_description);
    shuttle.Shuttle("node", m_node);
    if(Beam::Serialization::IsReceiver<Shuttler>::value) {
      m_icon = QIcon(QString::fromStdString(m_iconPath));
      Validate();
    }
  }
}

#endif
