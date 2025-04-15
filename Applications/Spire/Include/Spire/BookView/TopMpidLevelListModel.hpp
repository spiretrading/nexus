#ifndef SPIRE_TOP_MPID_LEVEL_LIST_MODEL_HPP
#define SPIRE_TOP_MPID_LEVEL_LIST_MODEL_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Stores the price of an MPID's top-most level. */
  struct TopMpidLevel {

    /** The MPID's market. */
    Nexus::MarketCode m_market;

    /** The index of the top level within the list of book quotes. */
    int m_index;

    /** The price of the represented MPID's top-most level. */
    Nexus::Money m_price;
  };

  /**
   * Maintains a list of the top MPIDs for all represented markets in a book.
   */
  class TopMpidLevelListModel : public ListModel<TopMpidLevel> {
    public:

      /**
       * Constructs a TopMpidLevelListModel from a list of book quotes.
       * @param quotes The list of book quotes to keep track of top MPIDs for.
       */
      explicit TopMpidLevelListModel(
        std::shared_ptr<BookQuoteListModel> quotes);

      int get_size() const override;

      const Type& get(int index) const;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<BookQuoteListModel> m_quotes;
      ArrayListModel<TopMpidLevel> m_top_mpids;
      Nexus::BookQuote m_removed_quote;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const BookQuoteListModel::Operation& operation);
  };
}

#endif
