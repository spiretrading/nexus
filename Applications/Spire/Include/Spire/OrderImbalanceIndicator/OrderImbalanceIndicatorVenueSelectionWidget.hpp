#ifndef SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGET_HPP
#define SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGET_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"

class QCheckBox;

namespace Spire {

  /*! \class OrderImbalanceIndicatorVenueSelectionWidget
      \brief Displays the list of available venues to display OrderImbalances
             for.
   */
  class OrderImbalanceIndicatorVenueSelectionWidget : public QWidget,
      public LegacyUI::PersistentWindow {
    public:

      //! Constructs an OrderImbalanceIndicatorVenueSelectionWidget.
      /*!
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorVenueSelectionWidget(QWidget* parent = nullptr);

      //! Constructs an OrderImbalanceIndicatorVenueSelectionWidget.
      /*!
        \param model The model to update.
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorVenueSelectionWidget(
        Beam::Ref<OrderImbalanceIndicatorModel> model,
        QWidget* parent = nullptr);

      //! Initializes this widget.
      /*!
        \param model The model to update.
      */
      void Initialize(Beam::Ref<OrderImbalanceIndicatorModel> model);

      virtual std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const;

    private:
      OrderImbalanceIndicatorModel* m_model;
      std::vector<std::unique_ptr<QCheckBox>> m_checkBoxes;
      std::unordered_map<QCheckBox*, Nexus::Venue> m_venues;

      void OnCheckBoxStateChanged(int state);
  };
}

#endif
