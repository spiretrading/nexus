#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <QFrame>
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/SecurityContext.hpp"
#include "Spire/UI/SecurityViewStack.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/Utilities/Utilities.hpp"

class QStatusBar;
class Ui_TimeAndSalesWindow;

namespace Spire {

  /** Displays TimeAndSale prints. */
  class TimeAndSalesWindow : public QFrame, public UI::PersistentWindow,
      public UI::SecurityContext {
    public:

      /**
       * Constructs a TimeAndSalesWindow.
       * @param userProfile The user's profile.
       * @param properties The properties used to display this window.
       * @param identifier The SecurityContext's identifier.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      TimeAndSalesWindow(Beam::Ref<UserProfile> userProfile,
        const TimeAndSalesProperties& properties,
        const std::string& identifier = "", QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the properties used to display this window. */
      const TimeAndSalesProperties& GetProperties() const;

      /** Sets the properties used to display this window. */
      void SetProperties(const TimeAndSalesProperties& properties);

      /**
       * Sets the Security to display.
       * @param security The Security to display.
       */
      void DisplaySecurity(const Nexus::Security& security);

      std::unique_ptr<UI::WindowSettings> GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void HandleLink(SecurityContext& context) override;
      void HandleUnlink() override;

    private:
      friend class TimeAndSalesWindowSettings;
      std::unique_ptr<Ui_TimeAndSalesWindow> m_ui;
      UserProfile* m_userProfile;
      TimeAndSalesProperties m_properties;
      Nexus::Security m_security;
      UI::SecurityViewStack m_securityViewStack;
      QStatusBar* m_statusBar;
      UI::ValueLabel* m_volumeLabel;
      std::unique_ptr<TimeAndSalesModel> m_model;
      std::shared_ptr<SecurityTechnicalsModel> m_securityTechnicalsModel;
      boost::signals2::scoped_connection m_volumeConnection;
      boost::signals2::scoped_connection m_linkConnection;
      std::string m_linkIdentifier;

      void SetupSecurityTechnicals();
      void OnVolumeUpdate(Nexus::Quantity volume);
      void OnContextMenu(const QPoint& position);
      void OnSectionMoved(
        int logicalIndex, int oldVisualIndex, int newVisualIndex);
      void OnSectionResized(int logicalIndex, int oldSize, int newSize);
  };
}

#endif
