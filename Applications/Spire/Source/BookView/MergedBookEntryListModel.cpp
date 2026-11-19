#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include <algorithm>

using namespace boost;
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
  m_transaction.set_size(get_size());
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

template<typename T>
void MergedBookEntryListModel::apply(const ListModel<T>::Operation& operation,
    const ListModel<T>& source, int offset) {
  visit(operation,
    [&] (ListModel<T>::StartTransaction) {
      m_transaction.start();
    },
    [&] (ListModel<T>::EndTransaction) {
      m_transaction.end();
    },
    [&] (const ListModel<T>::AddOperation& operation) {
      auto index = offset + operation.m_index;
      m_entries.insert(
        std::next(m_entries.begin(), index), source.get(operation.m_index));
      m_transaction.push(AddOperation(index));
    },
    [&] (const ListModel<T>::PreRemoveOperation& operation) {
      m_transaction.push(PreRemoveOperation(offset + operation.m_index));
    },
    [&] (const ListModel<T>::RemoveOperation& operation) {
      auto index = offset + operation.m_index;
      m_entries.erase(std::next(m_entries.begin(), index));
      m_transaction.push(RemoveOperation(index));
    },
    [&] (const ListModel<T>::MoveOperation& operation) {
      auto source_index = offset + operation.m_source;
      auto destination = offset + operation.m_destination;
      move_book_entry(m_entries, source_index, destination);
      m_transaction.push(MoveOperation(source_index, destination));
    },
    [&] (const ListModel<T>::UpdateOperation& operation) {
      auto index = offset + operation.m_index;
      m_entries[index] = operation.get_value();
      m_transaction.push(UpdateOperation(
        index, operation.get_previous(), operation.get_value()));
    });
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
  if(!preview && !m_previous_preview) {
    return;
  }
  auto index = m_book_quotes->get_size() + m_user_orders->get_size();
  if(!preview) {
    m_transaction.transact([&] {
      m_transaction.push(PreRemoveOperation(index));
      m_previous_preview = none;
      m_entries.pop_back();
      m_transaction.push(RemoveOperation(index));
    });
  } else if(m_previous_preview) {
    auto update = UpdateOperation(index, *m_previous_preview, *preview);
    m_previous_preview = preview;
    m_entries[index] = *preview;
    m_transaction.push(update);
  } else {
    m_previous_preview = preview;
    m_entries.push_back(*preview);
    m_transaction.push(AddOperation(index));
  }
}

void Spire::move_book_entry(
    std::deque<BookEntry>& entries, int source, int destination) {
  auto i = std::next(entries.begin(), source);
  auto j = std::next(entries.begin(), destination);
  if(source < destination) {
    std::rotate(i, std::next(i), std::next(j));
  } else {
    std::rotate(j, i, std::next(i));
  }
}
