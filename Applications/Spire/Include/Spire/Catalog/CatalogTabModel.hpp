#ifndef SPIRE_CATALOGTABMODEL_HPP
#define SPIRE_CATALOGTABMODEL_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractListModel>
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire {

  /*! \class CatalogTabModel
      \brief Contains the model for a CatalogTabView.
   */
  class CatalogTabModel : public QAbstractListModel {
    public:

      //! Signals a change in the name.
      typedef boost::signals2::signal<void ()> NameSignal;

      //! Constructs a CatalogTabModel.
      CatalogTabModel();

      //! Returns the name of this tab.
      const std::string& GetName() const;

      //! Sets the name of this tab.
      void SetName(const std::string& name);

      //! Returns all CatalogEntries represented by this model.
      const std::vector<CatalogEntry*>& GetEntries() const;

      //! Adds a CatalogEntry to this tab.
      /*!
        \param entry The CatalogEntry to add.
      */
      void Add(Beam::Ref<CatalogEntry> entry);

      //! Removes a CatalogEntry from this tab.
      /*!
        \param entry The CatalogEntry to remove.
      */
      void Remove(const CatalogEntry& entry);

      //! Connects a slot to the NameSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectNameSignal(
        const NameSignal::slot_type& slot) const;

      virtual Qt::DropActions supportedDropActions() const;

      virtual Qt::ItemFlags flags(const QModelIndex &index) const;

      virtual int rowCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

      virtual QStringList mimeTypes() const;

      virtual QMimeData* mimeData(const QModelIndexList& indexes) const;

      virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action,
        int row, int column, const QModelIndex& parent);

    private:
      std::string m_name;
      std::vector<CatalogEntry*> m_entries;
      mutable NameSignal m_nameSignal;
  };
}

#endif
