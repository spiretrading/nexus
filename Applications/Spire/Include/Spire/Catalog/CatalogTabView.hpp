#ifndef SPIRE_CATALOGTAB_HPP
#define SPIRE_CATALOGTAB_HPP
#include <set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>
#include <QItemSelectionModel>
#include <QWidget>
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/Catalog/CatalogSearchBarModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

class QModelIndex;
class QSortFilterProxyModel;
class Ui_CatalogTabView;

namespace Spire {

  /*! \class CatalogTabView
      \brief Displays the contents of a single tab in a CatalogWindow.
   */
  class CatalogTabView : public QWidget {
    public:

      //! Signals a CatalogEntry was activated.
      typedef boost::signals2::signal<void ()> CatalogEntryActivatedSignal;

      //! Specifies the filter to apply to CatalogEntries.
      /*!
        \param entry The CatalogEntry to test.
        \return <code>true</code> iff the <i>entry</i> should be displayed.
      */
      typedef std::function<bool (const CatalogEntry& entry)> Filter;

      //! Constructs a CatalogTabView.
      /*!
        \param model The CatalogTabModel to display.
        \param searchBarModel The model used for the search bar.
        \param filter A function used to filter out CatalogEntries.
        \param userProfile The UserProfile.
        \param saving <code>true</code> to display save related icons.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogTabView(Beam::Ref<CatalogTabModel> model,
        Beam::Ref<CatalogSearchBarModel> searchBarModel,
        const Filter& filter, Beam::Ref<UserProfile> userProfile,
        bool saving, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      //! Returns the model being displayed.
      CatalogTabModel& GetModel();

      //! Returns the model being displayed.
      const CatalogTabModel& GetModel() const;

      //! Returns the names currently displayed in the text bar.
      std::vector<std::string> GetEntryNames() const;

      //! Returns the CatalogEntries currently selected.
      std::vector<CatalogEntry*> GetSelection() const;

      //! Gives this view the focus.
      void GiveFocus();

      //! Sends a QDropEvent received by a tab to this view.
      void SendTabDropEvent(QDropEvent& event);

      //! Connects a slot to the CatalogEntryActivatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectCatalogEntryActivatedSignal(
        const CatalogEntryActivatedSignal::slot_type& slot) const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);
      virtual void hideEvent(QHideEvent* event);

    private:
      std::unique_ptr<Ui_CatalogTabView> m_ui;
      CatalogTabModel* m_model;
      CatalogSearchBarModel* m_searchBarModel;
      boost::signals2::scoped_connection m_searchBarModelConnection;
      QSortFilterProxyModel* m_proxyModel;
      UserProfile* m_userProfile;
      bool m_isAllTab;
      bool m_searchBarSelected;
      std::set<boost::uuids::uuid> m_modifiedEntries;
      mutable CatalogEntryActivatedSignal m_catalogEntryActivatedSignal;

      bool OnCatalogContextMenu(QContextMenuEvent* event);
      void OnSearchBarUpdate(CatalogSearchBarModel::Mode mode,
        const std::string& text);
      void Delete(const std::vector<CatalogEntry*>& entries);
      void OnSearchTextEdited(const QString& text);
      void OnSearchTextReturn();
      void OnSelectionChanged(const QItemSelection& selected,
        const QItemSelection& deselected);
      void OnActivated(const QModelIndex& index);
      void OnDescriptionChanged();
  };
}

#endif
