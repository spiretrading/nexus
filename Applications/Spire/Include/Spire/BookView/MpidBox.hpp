#ifndef SPIRE_MPID_BOX_HPP
#define SPIRE_MPID_BOX_HPP
#include <boost/optional/optional.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
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
  using UserOrderRow = StateSelector<void, struct UserOrderSelectorTag>;

  /** Styles an MpidBox based on whether it represents an order preview. */
  using PreviewRow = StateSelector<void, struct PreviewSelectorTag>;
}

  /** The type of model used for an MPID. */
  using MpidModel = ValueModel<Mpid>;

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
       * @param top_mpid_level The list of top levels by MPID.
       */
      MpidBox(std::shared_ptr<MpidModel> current,
        std::shared_ptr<ValueModel<int>> level,
        std::shared_ptr<ValueModel<bool>> is_top_mpid);

      /** Returns the displayed MPID. */
      std::shared_ptr<MpidModel> get_current() const;

      /** Returns the represented price level. */
      std::shared_ptr<ValueModel<int>> get_level() const;

      /** Returns whether this represents the top MPID price level. */
      std::shared_ptr<ValueModel<bool>> is_top_mpid() const;

    private:
      std::shared_ptr<MpidModel> m_current;
      boost::optional<Mpid::Origin> m_current_origin;
      Nexus::MarketCode m_current_market;
      std::shared_ptr<ValueModel<int>> m_level;
      int m_current_level;
      std::shared_ptr<ValueModel<bool>> m_is_top_mpid;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_level_connection;
      boost::signals2::scoped_connection m_is_top_mpid_connection;

      QString make_id(const Mpid& mpid) const;
      void on_current(const Mpid& mpid);
      void on_level(int level);
      void on_is_top_mpid(bool is_top);
  };
}

#endif
