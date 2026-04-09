#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <QFrame>
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/TickerContext.hpp"
#include "Spire/UI/TickerViewStack.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/Utilities/Utilities.hpp"

class QStatusBar;
class Ui_TimeAndSalesWindow;

namespace Spire {

  /** Displays TimeAndSale prints. */
  class TimeAndSalesWindow : public QFrame, public UI::PersistentWindow,
      public UI::TickerContext {
    public:

      /**
       * Constructs a TimeAndSalesWindow.
       * @param userProfile The user's profile.
       * @param properties The properties used to display this window.
       * @param identifier The TickerContext's identifier.
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
       * Sets the Ticker to display.
       * @param ticker The Ticker to display.
       */
      void DisplayTicker(const Nexus::Ticker& ticker);

      std::unique_ptr<UI::WindowSettings> GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void HandleLink(TickerContext& context) override;
      void HandleUnlink() override;

    private:
      friend class TimeAndSalesWindowSettings;
      std::unique_ptr<Ui_TimeAndSalesWindow> m_ui;
      UserProfile* m_userProfile;
      TimeAndSalesProperties m_properties;
      Nexus::Ticker m_ticker;
      UI::TickerViewStack m_tickerViewStack;
      QStatusBar* m_statusBar;
      UI::ValueLabel* m_volumeLabel;
      std::unique_ptr<TimeAndSalesModel> m_model;
      std::shared_ptr<TickerTechnicalsModel> m_tickerTechnicalsModel;
      boost::signals2::scoped_connection m_volumeConnection;
      boost::signals2::scoped_connection m_linkConnection;
      std::string m_linkIdentifier;

      void SetupTickerTechnicals();
      void OnVolumeUpdate(Nexus::Quantity volume);
      void OnContextMenu(const QPoint& position);
      void OnSectionMoved(
        int logicalIndex, int oldVisualIndex, int newVisualIndex);
      void OnSectionResized(int logicalIndex, int oldSize, int newSize);
  };
}

#endif
