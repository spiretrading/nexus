#ifndef SPIRE_DASHBOARDWINDOW_HPP
#define SPIRE_DASHBOARDWINDOW_HPP
#include <memory>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/CanvasView/OrderTaskView.hpp"
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"

class Ui_DashboardWindow;

namespace Spire {

  /*! \class DashboardWindow
      \brief A QWidget that displays a DashboardModel.
   */
  class DashboardWindow : public QWidget, public LegacyUI::PersistentWindow {
    public:

      //! Returns the name of a new DashboardWindow.
      static std::string GetDefaultName();

      //! Constructs a DashboardWindow.
      /*!
        \param name The name of the dashboard to display.
        \param schema The schema representing the model to display.
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      DashboardWindow(const std::string& name,
        const DashboardModelSchema& schema,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~DashboardWindow();

      virtual std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const;

    protected:
      virtual void showEvent(QShowEvent* event);
      virtual void closeEvent(QCloseEvent* event);
      virtual void keyPressEvent(QKeyEvent* event);

    private:
      friend class DashboardWindowSettings;
      struct BboQuoteEntry {
        int m_counter = 0;
        std::shared_ptr<Beam::StateQueue<Nexus::BboQuote>> m_bboQuote;
      };
      std::unique_ptr<Ui_DashboardWindow> m_ui;
      std::string m_name;
      UserProfile* m_userProfile;
      std::unique_ptr<DashboardModel> m_model;
      boost::optional<OrderTaskView> m_orderTaskView;
      std::unordered_map<Nexus::Security, BboQuoteEntry> m_bboQuotes;
      boost::signals2::scoped_connection m_rowAddedConnection;
      boost::signals2::scoped_connection m_rowRemovedConnection;
      boost::signals2::scoped_connection m_dashboardAddedConnection;
      boost::signals2::scoped_connection m_dashboardRemovedConnection;

      boost::optional<Nexus::Security> GetActiveSecurity() const;
      void Save();
      void SetName(const std::string& name);
      void Apply(const DashboardModelSchema& schema, const std::string& name);
      void OnRowAdded(const DashboardRow& row);
      void OnRowRemoved(const DashboardRow& row);
      void OnDashboardAdded(const SavedDashboards::Entry& entry);
      void OnDashboardRemoved(const SavedDashboards::Entry& entry);
      void OnDashboardActivated(const QString& text);
      void OnSaveButtonPressed();
      void OnDeleteButtonPressed();
  };
}

#endif
