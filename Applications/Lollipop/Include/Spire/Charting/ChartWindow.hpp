#ifndef SPIRE_CHART_WINDOW_HPP
#define SPIRE_CHART_WINDOW_HPP
#include <optional>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QMainWindow>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartInteractions.hpp"
#include "Spire/Charting/ChartPlotController.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/TickerContext.hpp"
#include "Spire/UI/TickerViewStack.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/UI/WindowSettings.hpp"

class QMenu;
class Ui_ChartWindow;

namespace Spire {

  /** Displays a chart. */
  class ChartWindow : public QMainWindow, public UI::PersistentWindow,
      public UI::TickerContext {
    public:

      /**
       * Constructs a ChartWindow.
       * @param userProfile The user's profile.
       * @param identifier The TickerContext's identifier.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit ChartWindow(Beam::Ref<UserProfile> userProfile,
        const std::string& identifier = "", QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the ChartInteractionMode. */
      ChartInteractionMode GetInteractionMode() const;

      /** Sets the interaction mode. */
      void SetInteractionMode(ChartInteractionMode interactionMode);

      /** Returns <code>true</code> iff auto-scale is enabled. */
      bool IsAutoScaleEnabled() const;

      /** Sets whether auto-scale is enabled. */
      void SetAutoScale(bool autoScale);

      /** Returns <code>true</code> iff the grid is locked. */
      bool IsLockGridEnabled() const;

      /** Sets whether the grid is locked. */
      void SetLockGrid(bool lockGrid);

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
      friend class ChartWindowSettings;
      std::unique_ptr<Ui_ChartWindow> m_ui;
      QMenu* m_linkMenu;
      ChartIntervalComboBox* m_intervalComboBox;
      UserProfile* m_userProfile;
      ChartInteractionMode m_interactionMode;
      std::optional<ChartPlotController> m_controller;
      Nexus::Ticker m_ticker;
      std::string m_linkIdentifier;
      UI::TickerViewStack m_tickerViewStack;
      ChartValue m_xPan;
      ChartValue m_yPan;
      boost::signals2::scoped_connection m_linkConnection;
      boost::signals2::scoped_connection m_verticalSliderConnection;
      boost::signals2::scoped_connection m_horizontalSliderConnection;
      boost::signals2::scoped_connection m_intervalChangedConnection;
      boost::signals2::scoped_connection m_beginPanConnection;
      boost::signals2::scoped_connection m_endPanConnection;
      EventHandler m_eventHandler;

      void AdjustSlider(int previousMinimum, int previousMaximum, int minimum,
        int maximum, UI::ScalableScrollBar* scrollBar);
      void UpdateInteractionMode();
      void OnVerticalSliderChanged(
        int previousMinimum, int previousMaximum, int minimum, int maximum);
      void OnHorizontalSliderChanged(
        int previousMinimum, int previousMaximum, int minimum, int maximum);
      void OnIntervalChanged(
        const std::shared_ptr<NativeType>& type, ChartValue value);
      void OnBeginPan();
      void OnEndPan();
      void OnPanActionToggled(bool toggled);
      void OnZoomActionToggled(bool toggled);
      void OnAutoScaleActionToggled(bool toggled);
      void OnLockGridActionToggled(bool toggled);
      void OnLinkMenuActionTriggered(bool triggered);
      void OnLinkActionTriggered(QAction* action);
  };
}

#endif
