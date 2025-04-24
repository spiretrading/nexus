#ifndef SPIRE_CURRENT_USER_ORDER_MODEL_HPP
#define SPIRE_CURRENT_USER_ORDER_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/SortedTableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/TableCurrentController.hpp"

namespace Spire {

  /** Stores the current UserOrder. */
  struct CurrentUserOrder {

    /** The current UserOrder. */
    BookViewModel::UserOrder m_user_order;

    /** The side of the current UserOrder. */
    Nexus::Side m_side;
  };

  /**
   * Keeps track of the current UserOrder between both sides of the book quotes.
   */
  class CurrentUserOrderModel : ValueModel<boost::optional<CurrentUserOrder>> {
    public:

      /**
       * Constructs a CurrentUserOrderModel by consolidating the current bid
       * with the current ask.
       */
      CurrentUserOrderModel(std::shared_ptr<SortedTableModel> bid_table,
        std::shared_ptr<TableCurrentController::CurrentModel> current_bid,
        std::shared_ptr<SortedTableModel> ask_table,
        std::shared_ptr<TableCurrentController::CurrentModel> current_ask);

      /**
       * If the current UserOrder is a bid, then the current is switched to the
       * UserOrder on the ask whose row is closest.
       */
      void navigate_to_bids();

      /**
       * If the current UserOrder is an ask, then the current is switched to the
       * UserOrder on the bid whose row is closest.
       */
      void navigate_to_asks();

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      struct SideEntry {
        std::shared_ptr<SortedTableModel> m_table;
        std::shared_ptr<TableCurrentController::CurrentModel> m_current;
        boost::signals2::scoped_connection m_connection;
      };
      SideEntry m_current_bid;
      SideEntry m_current_ask;
      boost::optional<TableRowIndexTracker> m_undo_navigation;
      LocalValueModel<boost::optional<CurrentUserOrder>> m_current;
      boost::signals2::scoped_connection m_undo_navigation_connection;

      void update_side(const SideEntry& selected_side,
        const SideEntry& other_side, Nexus::Side side,
        const boost::optional<TableIndex>& current_index);
      void on_bid(const boost::optional<TableIndex>& current_bid);
      void on_ask(const boost::optional<TableIndex>& current_ask);
      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
