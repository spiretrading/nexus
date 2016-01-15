#ifndef SPIRE_USERCATALOGENTRY_HPP
#define SPIRE_USERCATALOGENTRY_HPP
#include <boost/filesystem/path.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Catalog/PersistentCatalogEntry.hpp"

namespace Spire {

  /*! \class UserCatalogEntry
      \brief CatalogEntry defined by the user.
   */
  class UserCatalogEntry : public PersistentCatalogEntry {
    public:

      //! Returns the meta-data value used to indicate the CanvasNode's source.
      static const std::string& GetUserSourceValue();

      //! Constructs a UserCatalogEntry.
      /*!
        \param catalogPath The path to save this entry to.
      */
      UserCatalogEntry(const boost::filesystem::path& catalogPath);

      //! Constructs a UserCatalogEntry.
      /*!
        \param name The name of the entry.
        \param node The CanvasNode to represent.
        \param iconPath The file path to the entry's icon.
        \param description The description of the entry.
        \param catalogPath The path to save this entry to.
      */
      UserCatalogEntry(const std::string& name, const CanvasNode& node,
        const std::string& iconPath, const std::string& description,
        const boost::filesystem::path& catalogPath);

      //! Constructs a UserCatalogEntry.
      /*!
        \param name The name of the entry.
        \param node The CanvasNode to represent.
        \param uuid The entry's UUID.
        \param iconPath The file path to the entry's icon.
        \param description The description of the entry.
        \param catalogPath The path to save this entry to.
      */
      UserCatalogEntry(const std::string& name, const CanvasNode& node,
        const boost::uuids::uuid& uuid, const std::string& iconPath,
        const std::string& description,
        const boost::filesystem::path& catalogPath);

      virtual ~UserCatalogEntry();

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
      std::string m_name;
      std::unique_ptr<CanvasNode> m_node;
      std::string m_iconPath;
      QIcon m_icon;
      std::string m_description;
      boost::filesystem::path m_catalogPath;

      void Validate();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void UserCatalogEntry::Shuttle(Shuttler& shuttle, unsigned int version) {
    CatalogEntry::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("description", m_description);
    shuttle.Shuttle("icon_path", m_iconPath);
    shuttle.Shuttle("node", m_node);
    if(Beam::Serialization::IsReceiver<Shuttler>::value) {
      m_icon = QIcon(QString::fromStdString(m_iconPath));
      Validate();
    }
  }
}

#endif
