#ifndef SPIRE_MARKET_LIST_ITEM_HPP
#define SPIRE_MARKET_LIST_ITEM_HPP
#include <QListWidget>
#include "Nexus/Definitions/Market.hpp"
#include "spire/book_view/book_view_properties.hpp"

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

      //! Returns the market highlight associated with this item.
      const boost::optional<book_view_properties::market_highlight>&
        get_market_highlight() const;

      //! Sets the market highlight's color. If the market highlight is
      //! uninitialized, the market highlight is set to highlight the top
      //! level.
      void set_highlight_color(const QColor& color);

      //! Sets the market highlight to highlight all levels.
      void set_highlight_all_levels();

      //! Sets the market highlight to highlight the top level only.
      void set_highlight_top_level();

      //! Removes the market highlight.
      void remove_highlight();

    private:
      boost::optional<book_view_properties::market_highlight>
        m_market_highlight;
      Nexus::MarketDatabase::Entry m_market;

      void initialize();
  };
}

#endif
