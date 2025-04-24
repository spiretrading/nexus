#ifndef SPIRE_MPID_BOX_HPP
#define SPIRE_MPID_BOX_HPP
#include <boost/optional/optional.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Styles/StateSelector.hpp"

namespace Spire {
namespace Styles {

  /** Styles an MpidBox based on its price level. */
  using PriceLevelRow = StateSelector<int, struct PriceLevelRowSelectorTag>;

  /** Styles an MpidBox based on its market. */
  using MarketRow =
    StateSelector<Nexus::MarketCode, struct MarketRowSelectorTag>;

  /** Styles an MpidBox based on whether it's the top quote for its market. */
  using TopMarketRow = StateSelector<void, struct TopMarketRowSelectorTag>;

  /** Styles an MpidBox based on whether it represents a user's order. */
  using UserOrderRow =
    StateSelector<Nexus::OrderStatus, struct UserOrderSelectorTag>;

  /** Styles an MpidBox based on whether it represents an order preview. */
  using PreviewRow = StateSelector<void, struct PreviewSelectorTag>;
}

  /**
   * Displays an MPID in a TableView. The MpidBox can also be used to style the
   * row in a TableView based on the style state that is matched.
   */
  class MpidBox : public QWidget {
    public:

      /**
       * Constructs an MpidBox.
       * @param current The MPID to display.
       * @param level The price level represented.
       * @param is_top_mpid Whether this listing is the top price for the
       *        current MPID.
       */
      MpidBox(std::shared_ptr<BookEntryModel> current,
        std::shared_ptr<ValueModel<int>> level,
        std::shared_ptr<ValueModel<bool>> is_top_mpid);

      /** Returns the displayed MPID. */
      const std::shared_ptr<BookEntryModel>& get_current() const;

      /** Returns the represented price level. */
      const std::shared_ptr<ValueModel<int>>& get_level() const;

      /** Returns whether this represents the top MPID price level. */
      const std::shared_ptr<ValueModel<bool>>& is_top_mpid() const;

    protected:
      bool event(QEvent* event) override;

    private:
      std::shared_ptr<BookEntryModel> m_current;
      boost::optional<int> m_current_type_index;
      Nexus::MarketCode m_current_market;
      Nexus::OrderStatus m_current_status;
      std::shared_ptr<ValueModel<int>> m_level;
      int m_current_level;
      std::shared_ptr<ValueModel<bool>> m_is_top_mpid;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_level_connection;
      boost::signals2::scoped_connection m_is_top_mpid_connection;

      void update_row_state(int type_index);
      void update_market_state(const BookEntry& entry);
      void update_status(const BookEntry& entry);
      void on_current(const BookEntry& entry);
      void on_level(int level);
      void on_is_top_mpid(bool is_top);
  };
}

#endif
