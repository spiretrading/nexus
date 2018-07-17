#ifndef SPIRE_MARKET_LIST_ITEM_HPP
#define SPIRE_MARKET_LIST_ITEM_HPP
#include <QListWidget>
#include "Nexus/Definitions/Market.hpp"

namespace spire {

  //! Displays a market's code as a list item.
  class market_list_item : public QListWidgetItem {
    public:

      //! Constructs a market_list_item and sets it's display text to the
      //! market code.
      /*
        \param market The market that this widget represents.
        \param parent The list that this widget will be displayed in.
      */
      market_list_item(const Nexus::MarketDatabase::Entry& market,
        QListWidget* parent = nullptr);

      //! Returns the market info associated with this item.
      const Nexus::MarketDatabase::Entry& get_market_info() const;

      boost::optional<const book_view_properties::market_highlight&>
        get_market_highlight() const;

    private:
      boost::optional<book_view_properties::market_highlight>
        m_market_highlight;
      Nexus::MarketDatabase::Entry m_market;
  };
}

#endif
