#ifndef SPIRE_USER_CATALOG_ENTRY_HPP
#define SPIRE_USER_CATALOG_ENTRY_HPP
#include <filesystem>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Catalog/PersistentCatalogEntry.hpp"

namespace Spire {

  /** CatalogEntry defined by the user. */
  class UserCatalogEntry : public PersistentCatalogEntry {
    public:

      /**
       * Returns the meta-data value used to indicate the CanvasNode's source.
       */
      static const std::string& GetUserSourceValue();

      /**
       * Constructs a UserCatalogEntry.
       * @param catalogPath The path to save this entry to.
       */
      explicit UserCatalogEntry(const std::filesystem::path& catalogPath);

      /**
       * Constructs a UserCatalogEntry.
       * @param name The name of the entry.
       * @param node The CanvasNode to represent.
       * @param iconPath The file path to the entry's icon.
       * @param description The description of the entry.
       * @param catalogPath The path to save this entry to.
       */
      UserCatalogEntry(const std::string& name, const CanvasNode& node,
        const std::string& iconPath, const std::string& description,
        const std::filesystem::path& catalogPath);

      /**
       * Constructs a UserCatalogEntry.
       * @param name The name of the entry.
       * @param node The CanvasNode to represent.
       * @param uuid The entry's UUID.
       * @param iconPath The file path to the entry's icon.
       * @param description The description of the entry.
       * @param catalogPath The path to save this entry to.
       */
      UserCatalogEntry(const std::string& name, const CanvasNode& node,
        const boost::uuids::uuid& uuid, const std::string& iconPath,
        const std::string& description,
        const std::filesystem::path& catalogPath);

      void Save() const override;

      void Delete() const override;

      std::string GetName() const override;

      std::unique_ptr<CatalogEntry>
        SetName(const std::string& name) const override;

      const std::string& GetIconPath() const override;

      std::unique_ptr<CatalogEntry>
        SetIconPath(const std::string& iconPath) const override;

      QIcon GetIcon() const override;

      std::string GetDescription() const override;

      const CanvasNode& GetNode() const override;

      std::unique_ptr<CatalogEntry>
        SetNode(const CanvasNode& node) const override;

      bool IsReadOnly() const override;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      std::unique_ptr<CanvasNode> m_node;
      std::string m_iconPath;
      QIcon m_icon;
      std::string m_description;
      std::filesystem::path m_catalogPath;

      void Validate();
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void UserCatalogEntry::shuttle(S& shuttle, unsigned int version) {
    CatalogEntry::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("description", m_description);
    shuttle.shuttle("icon_path", m_iconPath);
    shuttle.shuttle("node", m_node);
    if(Beam::IsReceiver<S>) {
      m_icon = QIcon(QString::fromStdString(m_iconPath));
      Validate();
    }
  }
}

#endif
