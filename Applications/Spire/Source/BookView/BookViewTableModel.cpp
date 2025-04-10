#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/MergedBookQuoteListModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  struct ListingExtractor {
    MpidListing m_mpid;

    AnyRef operator ()(const BookListing& listing, int index) {
      auto column = static_cast<BookViewColumn>(index);
      if(auto quote = get<BookQuote>(&listing)) {
        if(column == BookViewColumn::MPID) {
          m_mpid = MpidListing(ListingSource::BOOK_QUOTE, quote->m_mpid);
          return std::as_const(m_mpid);
        } else if(column == BookViewColumn::PRICE) {
          return quote->m_quote.m_price;
        }
        return quote->m_quote.m_size;
      } else if(auto order = get<BookViewModel::UserOrder>(&listing)) {
        if(column == BookViewColumn::MPID) {
          m_mpid = MpidListing(ListingSource::USER_ORDER, order->m_destination);
          return std::as_const(m_mpid);
        } else if(column == BookViewColumn::PRICE) {
          return order->m_price;
        }
        return order->m_size;
      }
      auto preview = get<OrderFields>(&listing);
      if(column == BookViewColumn::MPID) {
        m_mpid = MpidListing(ListingSource::PREVIEW, preview->m_destination);
        return std::as_const(m_mpid);
      } else if(column == BookViewColumn::PRICE) {
        return preview->m_price;
      }
      return preview->m_quantity;
    }
  };
}

bool Spire::MpidListing::operator <(const MpidListing& listing) const {
  return m_mpid < listing.m_mpid;
}

std::shared_ptr<TableModel> Spire::make_book_view_table_model(
    std::shared_ptr<ListModel<BookListing>> listings) {
  return std::make_shared<ListToTableModel<BookListing>>(
    std::move(listings), BOOK_VIEW_COLUMN_SIZE, ListingExtractor());
}
