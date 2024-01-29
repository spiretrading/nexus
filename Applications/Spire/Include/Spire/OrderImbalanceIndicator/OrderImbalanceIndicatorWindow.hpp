#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_WINDOW_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include <QFrame>
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Utilities/Utilities.hpp"

class Ui_OrderImbalanceIndicatorWindow;

namespace Spire {

  /** Displays Market OrderImbalances. */
  class OrderImbalanceIndicatorWindow :
      public QFrame, public LegacyUI::PersistentWindow {
    public:

      /**
       * Constructs an OrderImbalanceIndicatorWindow.
       * @param userProfile The user's profile.
       * @param model The model to display.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      OrderImbalanceIndicatorWindow(Beam::Ref<UserProfile> userProfile,
        const std::shared_ptr<OrderImbalanceIndicatorModel>& model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the model used. */
      const std::shared_ptr<OrderImbalanceIndicatorModel>& GetModel() const;

      /**
       * Sets the model to represent.
       * @param model The model to represent.
       */
      void SetModel(const std::shared_ptr<OrderImbalanceIndicatorModel>& model);

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      friend class OrderImbalanceIndicatorWindowSettings;
      std::unique_ptr<Ui_OrderImbalanceIndicatorWindow> m_ui;
      UserProfile* m_userProfile;
      std::shared_ptr<OrderImbalanceIndicatorModel> m_model;
      std::unique_ptr<LegacyUI::CustomVariantSortFilterProxyModel> m_proxyModel;
      boost::signals2::scoped_connection m_timeRangeConnection;

      void OnTimeRangeUpdated(
        const TimeRangeParameter& startTime, const TimeRangeParameter& endTime);
  };
}

#endif
