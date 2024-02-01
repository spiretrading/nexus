#ifndef SPIRE_ORDERIMBALANCEINDICATORMARKETSELECTIONWIDGET_HPP
#define SPIRE_ORDERIMBALANCEINDICATORMARKETSELECTIONWIDGET_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"

class QCheckBox;

namespace Spire {

  /*! \class OrderImbalanceIndicatorMarketSelectionWidget
      \brief Displays the list of available markets to display OrderImbalances
             for.
   */
  class OrderImbalanceIndicatorMarketSelectionWidget : public QWidget,
      public LegacyUI::PersistentWindow {
    public:

      //! Constructs an OrderImbalanceIndicatorMarketSelectionWidget.
      /*!
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorMarketSelectionWidget(QWidget* parent = nullptr);

      //! Constructs an OrderImbalanceIndicatorMarketSelectionWidget.
      /*!
        \param marketDatabase The database of markets to display.
        \param model The model to update.
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorMarketSelectionWidget(
        const Nexus::MarketDatabase& marketDatabase,
        Beam::Ref<OrderImbalanceIndicatorModel> model,
        QWidget* parent = nullptr);

      //! Initializes this widget.
      /*!
        \param marketDatabase The database of markets to display.
        \param model The model to update.
      */
      void Initialize(const Nexus::MarketDatabase& marketDatabase,
        Beam::Ref<OrderImbalanceIndicatorModel> model);

      virtual std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const;

    private:
      OrderImbalanceIndicatorModel* m_model;
      std::vector<std::unique_ptr<QCheckBox>> m_checkBoxes;
      std::unordered_map<QCheckBox*, Nexus::MarketCode> m_markets;

      void OnCheckBoxStateChanged(int state);
  };
}

#endif
