#ifndef SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGET_HPP
#define SPIRE_ORDERIMBALANCEINDICATORVENUESELECTIONWIDGET_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/UI/PersistentWindow.hpp"

class QCheckBox;

namespace Spire {

  /*! \class OrderImbalanceIndicatorVenueSelectionWidget
      \brief Displays the list of available venues to display OrderImbalances
             for.
   */
  class OrderImbalanceIndicatorVenueSelectionWidget : public QWidget,
      public UI::PersistentWindow {
    public:

      //! Constructs an OrderImbalanceIndicatorVenueSelectionWidget.
      /*!
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorVenueSelectionWidget(QWidget* parent = nullptr);

      //! Constructs an OrderImbalanceIndicatorVenueSelectionWidget.
      /*!
        \param venueDatabase The database of venues to display.
        \param model The model to update.
        \param parent The parent widget.
      */
      OrderImbalanceIndicatorVenueSelectionWidget(
        const Nexus::VenueDatabase& venueDatabase,
        Beam::Ref<OrderImbalanceIndicatorModel> model,
        QWidget* parent = nullptr);

      //! Initializes this widget.
      /*!
        \param venueDatabase The database of venues to display.
        \param model The model to update.
      */
      void Initialize(const Nexus::VenueDatabase& venueDatabase,
        Beam::Ref<OrderImbalanceIndicatorModel> model);

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    private:
      OrderImbalanceIndicatorModel* m_model;
      std::vector<std::unique_ptr<QCheckBox>> m_checkBoxes;
      std::unordered_map<QCheckBox*, Nexus::Venue> m_venues;

      void OnCheckBoxStateChanged(int state);
  };
}

#endif
