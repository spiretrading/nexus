#ifndef SPIRE_IS_TOP_MPID_MODEL_HPP
#define SPIRE_IS_TOP_MPID_MODEL_HPP
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** Keeps track of whether an MPID for a given price is the top-most MPID. */
  class IsTopMpidModel : public ValueModel<bool> {
    public:

      /**
       * Constructs an IsTopMpidModel by checking if an MPID at a given price is
       * the top-most MPID based on a list of top MPID prices.
       * @param top_mpid_prices The list of top MPID prices.
       * @param entry The book entry whose venue is tracked.
       * @param price The price of the quote to track.
       */
      IsTopMpidModel(std::shared_ptr<ListModel<TopMpidPrice>> top_mpid_prices,
        std::shared_ptr<BookEntryModel> entry,
        std::shared_ptr<ValueModel<Nexus::Money>> price);

      const bool& get() const override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel<TopMpidPrice>> m_top_mpid_prices;
      std::shared_ptr<BookEntryModel> m_entry;
      std::shared_ptr<ValueModel<Nexus::Money>> m_price;
      Nexus::Venue m_venue;
      boost::optional<Nexus::Money> m_top_price;
      bool m_is_top_mpid_removed;
      LocalValueModel<bool> m_current;
      boost::signals2::scoped_connection m_entry_connection;
      boost::signals2::scoped_connection m_price_connection;
      boost::signals2::scoped_connection m_top_mpid_prices_connection;

      void initialize_top_price();
      void update_top_price(int index);
      void update_current();
      void on_entry(const BookEntry& entry);
      void on_price(Nexus::Money price);
      void on_operation(const ListModel<TopMpidPrice>::Operation& operation);
  };
}

#endif
