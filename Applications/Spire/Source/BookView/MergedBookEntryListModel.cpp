#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include <algorithm>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

MergedBookEntryListModel::MergedBookEntryListModel(
    std::shared_ptr<BookQuoteListModel> book_quotes,
    std::shared_ptr<BookViewModel::UserOrderListModel> user_orders,
    std::shared_ptr<BookViewModel::PreviewOrderModel> preview)
    : m_book_quotes(std::move(book_quotes)),
      m_user_orders(std::move(user_orders)),
      m_preview(std::move(preview)),
      m_previous_preview(m_preview->get()) {
  for(auto i = 0; i != m_book_quotes->get_size(); ++i) {
    m_entries.push_back(m_book_quotes->get(i));
  }
  for(auto i = 0; i != m_user_orders->get_size(); ++i) {
    m_entries.push_back(m_user_orders->get(i));
  }
  if(m_previous_preview) {
    m_entries.push_back(*m_previous_preview);
  }
  m_book_quotes_connection = m_book_quotes->connect_operation_signal(
    std::bind_front(&MergedBookEntryListModel::on_book_quote_operation, this));
  m_user_orders_connection = m_user_orders->connect_operation_signal(
    std::bind_front(&MergedBookEntryListModel::on_user_order_operation, this));
  m_preview_connection = m_preview->connect_update_signal(
    std::bind_front(&MergedBookEntryListModel::on_preview, this));
}

int MergedBookEntryListModel::get_size() const {
  return static_cast<int>(m_entries.size());
}

const MergedBookEntryListModel::Type&
    MergedBookEntryListModel::get(int index) const {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_entries[index];
}

connection MergedBookEntryListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void MergedBookEntryListModel::transact(
    const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

void MergedBookEntryListModel::move_entry(
    std::deque<BookEntry>& entries, int source, int destination) {
  auto i = std::next(entries.begin(), source);
  auto j = std::next(entries.begin(), destination);
  if(source < destination) {
    std::rotate(i, std::next(i), std::next(j));
  } else {
    std::rotate(j, i, std::next(i));
  }
}

void MergedBookEntryListModel::on_book_quote_operation(
    const BookQuoteListModel::Operation& operation) {
  apply<BookQuote>(operation, *m_book_quotes, 0);
}

void MergedBookEntryListModel::on_user_order_operation(
    const BookViewModel::UserOrderListModel::Operation& operation) {
  apply<BookViewModel::UserOrder>(
    operation, *m_user_orders, m_book_quotes->get_size());
}

void MergedBookEntryListModel::on_preview(
    const optional<OrderFields>& preview) {
  if(preview) {
    auto index = m_book_quotes->get_size() + m_user_orders->get_size();
    if(m_previous_preview) {
      auto update = UpdateOperation(index, *m_previous_preview, *preview);
      m_previous_preview = preview;
      m_entries[index] = *preview;
      m_transaction.push(update);
    } else {
      m_previous_preview = preview;
      m_entries.push_back(*preview);
      m_transaction.push(AddOperation(index));
    }
  } else if(m_previous_preview) {
    auto index = m_book_quotes->get_size() + m_user_orders->get_size();
    m_transaction.transact([&] {
      m_transaction.push(PreRemoveOperation(index));
      m_previous_preview = none;
      m_entries.pop_back();
      m_transaction.push(RemoveOperation(index));
    });
  }
}
