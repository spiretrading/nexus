#ifndef SPIRE_MARKET_COMBO_BOX_HPP
#define SPIRE_MARKET_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Markets.
  class MarketComboBox : public QLineEdit {
    public:

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

    private:
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
