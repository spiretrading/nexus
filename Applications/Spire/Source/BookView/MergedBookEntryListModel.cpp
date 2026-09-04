#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include <algorithm>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  void move_entry(
      std::vector<BookEntry>& entries, int source, int destination) {
    auto i = std::next(entries.begin(), source);
    auto j = std::next(entries.begin(), destination);
    if(source < destination) {
      std::rotate(i, std::next(i), std::next(j));
    } else {
      std::rotate(j, i, std::next(i));
    }
  }
}

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

void MergedBookEntryListModel::on_book_quote_operation(
    const BookQuoteListModel::Operation& operation) {
  visit(operation,
    [&] (BookQuoteListModel::StartTransaction) {
      m_transaction.start();
    },
    [&] (BookQuoteListModel::EndTransaction) {
      m_transaction.end();
    },
    [&] (const BookQuoteListModel::AddOperation& operation) {
      m_entries.insert(std::next(m_entries.begin(), operation.m_index),
        m_book_quotes->get(operation.m_index));
      m_transaction.push(AddOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::PreRemoveOperation& operation) {
      m_transaction.push(PreRemoveOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::RemoveOperation& operation) {
      m_entries.erase(std::next(m_entries.begin(), operation.m_index));
      m_transaction.push(RemoveOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::MoveOperation& operation) {
      move_entry(m_entries, operation.m_source, operation.m_destination);
      m_transaction.push(
        MoveOperation(operation.m_source, operation.m_destination));
    },
    [&] (const BookQuoteListModel::UpdateOperation& operation) {
      m_entries[operation.m_index] = operation.get_value();
      m_transaction.push(UpdateOperation(
        operation.m_index, operation.get_previous(), operation.get_value()));
    });
}

void MergedBookEntryListModel::on_user_order_operation(
    const BookViewModel::UserOrderListModel::Operation& operation) {
  visit(operation,
    [&] (BookViewModel::UserOrderListModel::StartTransaction) {
      m_transaction.start();
    },
    [&] (BookViewModel::UserOrderListModel::EndTransaction) {
      m_transaction.end();
    },
    [&] (const BookViewModel::UserOrderListModel::AddOperation& operation) {
      auto index = m_book_quotes->get_size() + operation.m_index;
      m_entries.insert(std::next(m_entries.begin(), index),
        m_user_orders->get(operation.m_index));
      m_transaction.push(AddOperation(index));
    },
    [&] (const
        BookViewModel::UserOrderListModel::PreRemoveOperation& operation) {
      m_transaction.push(
        PreRemoveOperation(m_book_quotes->get_size() + operation.m_index));
    },
    [&] (const BookViewModel::UserOrderListModel::RemoveOperation& operation) {
      auto index = m_book_quotes->get_size() + operation.m_index;
      m_entries.erase(std::next(m_entries.begin(), index));
      m_transaction.push(RemoveOperation(index));
    },
    [&] (const BookViewModel::UserOrderListModel::MoveOperation& operation) {
      auto source = m_book_quotes->get_size() + operation.m_source;
      auto destination = m_book_quotes->get_size() + operation.m_destination;
      move_entry(m_entries, source, destination);
      m_transaction.push(MoveOperation(source, destination));
    },
    [&] (const BookViewModel::UserOrderListModel::UpdateOperation& operation) {
      auto index = m_book_quotes->get_size() + operation.m_index;
      m_entries[index] = operation.get_value();
      m_transaction.push(UpdateOperation(
        index, operation.get_previous(), operation.get_value()));
    });
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
