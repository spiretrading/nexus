#ifndef SPIRE_CATALOGWINDOW_HPP
#define SPIRE_CATALOGWINDOW_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QDialog>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Catalog/Catalog.hpp"
#include "Spire/Catalog/CatalogSearchBarModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_CatalogWindow;

namespace Spire {

  /*! \class CatalogWindow
      \brief Used to save/load CatalogEntries.
   */
  class CatalogWindow : public QDialog {
    public:

      //! Specifies the filter to apply to CatalogEntries.
      /*!
        \param entry The CatalogEntry to test.
        \return <code>true</code> iff the <i>entry</i> should be displayed.
      */
      typedef std::function<bool (const CatalogEntry& entry)> Filter;

      //! A filter that accepts all CatalogEntries.
      static bool DisplayAllFilter(const CatalogEntry& entry);

      //! Returns a filter that tests a node's type for equality.
      /*!
        \param type The CanvasType that the CanvasNode must be equal to.
        \return A Filter that tests a node's type for equality with the
                specified <i>type</i>.
      */
      static Filter EqualTypeFilter(const CanvasType& type);

      //! Returns a filter that tests for nodes that can serve as substitutes.
      /*!
        \param node The CanvasNode being substituted.
        \return A Filter that tests for nodes that can serve as substitutes for
                the specified <i>node</i>.
      */
      static Filter SubstitutionFilter(const CanvasNode& node);

      //! Constructs a CatalogWindow for the purpose of loading CatalogEntries.
      /*!
        \param userProfile The UserProfile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogWindow(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a CatalogWindow for the purpose of saving a CanvasNode.
      /*!
        \param userProfile The UserProfile.
        \param node The CanvasNode to save.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogWindow(Beam::Ref<UserProfile> userProfile,
        const CanvasNode& node, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a CatalogWindow for the purpose of loading CatalogEntries.
      /*!
        \param userProfile The UserProfile.
        \param filter A function used to filter out CatalogEntries.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogWindow(Beam::Ref<UserProfile> userProfile,
        const Filter& filter, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a CatalogWindow for the purpose of saving a CanvasNode.
      /*!
        \param userProfile The UserProfile.
        \param node The CanvasNode to save.
        \param filter A function used to filter out CatalogEntries.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogWindow(Beam::Ref<UserProfile> userProfile,
        const Filter& filter, const CanvasNode& node, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~CatalogWindow();

      //! Returns the selected CatalogEntries.
      std::vector<CatalogEntry*> GetSelection() const;

      //! Returns the current tab.
      boost::optional<const CatalogTabView&> GetCurrentTab() const;

      //! Returns the current tab.
      boost::optional<CatalogTabView&> GetCurrentTab();

      //! Returns the names of the selected entries.
      std::vector<std::string> GetEntryNames() const;

      //! Returns the CatalogSettings used by this window.
      const CatalogSettings& GetSettings() const;

      //! Returns the CatalogSettings used by this window.
      CatalogSettings& GetSettings();

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      enum Mode {
        LOADING,
        SAVING
      };
      friend class CatalogTabWidget;
      class CatalogModel;
      std::unique_ptr<Ui_CatalogWindow> m_ui;
      Filter m_filter;
      Mode m_mode;
      UserProfile* m_userProfile;
      CatalogSearchBarModel m_searchBarModel;
      QWidget* m_newTab;
      boost::signals2::scoped_connection m_tabAddedConnection;
      boost::signals2::scoped_connection m_tabRemovedConnection;
      Beam::ConnectionGroup m_viewConnections;
      Beam::ConnectionGroup m_modelConnections;

      void Initialize();
      void SetDefaultSearchName(const CanvasNode& node);
      void OnDragMoveTab(QDragMoveEvent* event);
      void OnDropTab(QDropEvent* event);
      void OnNameChanged(const CatalogTabModel& model);
      void OnCatalogTabAdded(CatalogTabModel& model);
      void OnCatalogTabRemoved(CatalogTabModel& model);
      void AddTab(CatalogTabModel& model);
      void OnAccept();
      void OnCatalogEntryActivated();
  };
}

#endif
