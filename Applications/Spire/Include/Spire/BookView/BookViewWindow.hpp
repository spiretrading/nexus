#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <boost/signals2/connection.hpp>
#include <QFrame>
#include <QTimer>
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/SecurityContext.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Utilities/Utilities.hpp"

class QStatusBar;
class Ui_BookViewWindow;

namespace Spire {

  /** Displays a Security's BookQuotes. */
  class BookViewWindow : public QFrame, public LegacyUI::PersistentWindow,
      public LegacyUI::SecurityContext {
    public:

      /**
       * Constructs a BookViewWindow.
       * @param userProfile The user's profile.
       * @param properties The window's Properties.
       * @param identifier The SecurityContext identifier.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      BookViewWindow(Beam::Ref<UserProfile> userProfile,
        const BookViewProperties& properties,
        const std::string& identifier = "", QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the BookViewProperties used to display the book view. */
      const BookViewProperties& GetBookViewProperties() const;

      /** Sets the properties used to display the book view. */
      void SetProperties(const BookViewProperties& properties);

      /**
       * Sets the Security to display.
       * @param security The Security to display.
       */
      void DisplaySecurity(const Nexus::Security& security);

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void HandleLink(SecurityContext& context) override;
      void HandleUnlink() override;

    private:
      friend class BookViewWindowSettings;
      std::unique_ptr<Ui_BookViewWindow> m_ui;
      UserProfile* m_userProfile;
      BookViewProperties m_properties;
      Nexus::Security m_security;
      LegacyUI::SecurityViewStack m_securityViewStack;
      std::string m_linkIdentifier;
      boost::signals2::scoped_connection m_linkConnection;
      CondensedCanvasWidget* m_taskEntryWidget;
      bool m_isTaskEntryWidgetForInteractionsProperties;
      std::shared_ptr<SecurityTechnicalsModel> m_securityTechnicalsModel;
      Beam::SignalHandling::ConnectionGroup m_securityTechnicalsConnections;
      std::unordered_map<Nexus::Security, std::vector<std::shared_ptr<Task>>>
        m_tasksExecuted;
      bool m_bidPanelGuard;
      bool m_askPanelGuard;
      QTimer m_updateTimer;
      EventHandler m_eventHandler;

      void SetupSecurityTechnicalsModel();
      std::unique_ptr<CanvasNode> PrepareTaskNode(const CanvasNode& node);
      void RemoveTaskEntry();
      void ExecuteTask(const CanvasNode& node);
      void HandleSecurityInputEvent(QKeyEvent* event);
      void HandleKeyBindingEvent(const OrderTaskArguments& arguments);
      void HandleInteractionsPropertiesEvent();
      void HandleCancelBindingEvent(
        const CancelKeyBindingsModel::Operation& cancelBinding);
      void HandleTaskInputEvent(QKeyEvent* event);
      void UpdateDefaultQuantity();
      void OnOpenUpdate(Nexus::Money open);
      void OnCloseUpdate(Nexus::Money close);
      void OnHighUpdate(Nexus::Money high);
      void OnLowUpdate(Nexus::Money low);
      void OnVolumeUpdate(Nexus::Quantity volume);
      void OnTaskState(
        const std::shared_ptr<Task>& task, const Task::StateEntry& update);
      void OnContextMenu(const QPoint& position);
      void OnUpdateTimer();
  };
}

#endif
