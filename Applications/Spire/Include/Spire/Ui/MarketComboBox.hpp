#ifndef SPIRE_MARKET_COMBO_BOX_HPP
#define SPIRE_MARKET_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Markets.
  class MarketComboBox : public QWidget {
    public:

      //! Signals that a Market was selected.
      /*!
        \param market The MarketCode of the selected Market.
      */
      using SelectedSignal = Signal<void (Nexus::MarketCode market)>;

      //! Constructs a MarketComboBox.
      /*!
        \param database The source market database.
        \param parent The parent widget.
      */
      explicit MarketComboBox(const Nexus::MarketDatabase& database,
        QWidget* parent = nullptr);

      //! Returns the MarketCode of the current Market.
      Nexus::MarketCode get_market() const;

      //! Sets the MarketCode of the current Market.
      void set_market(Nexus::MarketCode market);

      //! Connects a slot to the Market selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
