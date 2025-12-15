#ifndef SPIRE_ACCOUNTVIEWMODEL_HPP
#define SPIRE_ACCOUNTVIEWMODEL_HPP
#include <memory>
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractItemModel>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class AccountViewModel
      \brief Stores the list of account entries.
   */
  class AccountViewModel : public QAbstractItemModel {
    public:

      //! The number of columns available.
      static const unsigned int COLUMN_COUNT = 1;

      //! Constructs an AccountViewModel.
      /*!
        \param userProfile The user's profile.
        \param parent The parent object.
      */
      AccountViewModel(Beam::Ref<UserProfile> userProfile,
        QObject* parent = nullptr);

      virtual ~AccountViewModel();

      //! Loads the model from the server.
      void Load();

      //! Returns the list of RootItems.
      const std::vector<RootItem*>& GetRoots() const;

      //! Returns the AccountViewItem at a specified index.
      /*!
        \param index The index of the AccountViewItem to retrieve.
        \return The AccountViewItem at the specified <i>index</i>.
      */
      AccountViewItem* GetItem(const QModelIndex& index) const;

      //! Returns the index of an AccountViewItem.
      /*!
        \param item The AccountViewItem whose index is to be retrieved.
        \return The index of the specified <i>item</i>.
      */
      QModelIndex GetIndex(const AccountViewItem& item) const;

      //! Expands the item at a specified index.
      /*!
        \param index The index of the item to expand.
      */
      void Expand(const QModelIndex& index);

      virtual Qt::ItemFlags flags(const QModelIndex& index) const;

      virtual QModelIndex index(int row, int column,
        const QModelIndex& parent) const;

      virtual QModelIndex parent(const QModelIndex& index) const;

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      UserProfile* m_userProfile;
      std::vector<std::unique_ptr<AccountViewItem>> m_items;
      RootItem* m_administratorsRoot;
      RootItem* m_servicesRoot;
      RootItem* m_tradingGroupsRoot;
      std::vector<RootItem*> m_roots;
      std::unordered_map<AccountViewItem*, std::vector<AccountViewItem*>>
        m_children;
      std::unordered_map<AccountViewItem*, AccountViewItem*> m_parents;

      void Add(std::unique_ptr<RootItem> root);
      void Add(std::unique_ptr<AccountViewItem> item, AccountViewItem* parent);
  };
}

#endif
