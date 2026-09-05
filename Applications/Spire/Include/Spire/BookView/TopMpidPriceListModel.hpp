#ifndef SPIRE_TOP_MPID_PRICE_LIST_MODEL_HPP
#define SPIRE_TOP_MPID_PRICE_LIST_MODEL_HPP
#include <unordered_map>
#include <boost/optional/optional.hpp>
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** Stores the price of an MPID's top level. */
  struct TopMpidPrice {

    /** The MPID's venue. */
    Nexus::Venue m_venue;

    /** The price of the represented MPID's top level. */
    Nexus::Money m_price;
  };

  /** Maintains a list of the top MPID prices for all venues in a book. */
  class TopMpidPriceListModel : public ListModel<TopMpidPrice> {
    public:

      /** The type of model representing a venue's top price. */
      using TopPriceModel = ValueModel<boost::optional<Nexus::Money>>;

      /**
       * Constructs a TopMpidPriceListModel from a list of book quotes.
       * @param quotes The list of book quotes to keep track of top MPIDs for.
       */
      explicit TopMpidPriceListModel(
        std::shared_ptr<BookQuoteListModel> quotes);

      /**
       * Returns the model of a venue's top price.
       * @param venue The venue whose top price is represented.
       */
      std::shared_ptr<TopPriceModel> get_top_price(Nexus::Venue venue);

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<BookQuoteListModel> m_quotes;
      ArrayListModel<TopMpidPrice> m_top_prices;
      std::unordered_map<Nexus::Venue, int> m_indexes;
      std::unordered_map<Nexus::Venue,
        std::shared_ptr<LocalValueModel<boost::optional<Nexus::Money>>>>
          m_top_price_models;
      Nexus::BookQuote m_removed_quote;
      boost::signals2::scoped_connection m_connection;

      boost::optional<int> find_index(Nexus::Venue venue) const;
      boost::optional<Nexus::Money> find_price(Nexus::Venue venue) const;
      void update_top_price_model(Nexus::Venue venue);
      void add_quote(const Nexus::BookQuote& quote);
      void remove_quote(const Nexus::BookQuote& quote);
      void on_operation(const BookQuoteListModel::Operation& operation);
  };
}

#endif
