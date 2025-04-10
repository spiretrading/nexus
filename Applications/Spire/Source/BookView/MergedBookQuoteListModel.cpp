#include "Spire/BookView/MergedBookQuoteListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

MergedBookQuoteListModel::MergedBookQuoteListModel(
    std::shared_ptr<BookQuoteListModel> book_quotes,
    std::shared_ptr<BookViewModel::UserOrderListModel> user_orders,
    std::shared_ptr<BookViewModel::PreviewOrderModel> preview)
    : m_book_quotes(std::move(book_quotes)),
      m_user_orders(std::move(user_orders)),
      m_preview(std::move(preview)),
      m_previous_preview(m_preview->get()) {
  m_book_quotes_connection = m_book_quotes->connect_operation_signal(
    std::bind_front(&MergedBookQuoteListModel::on_book_quote_operation, this));
  m_user_orders_connection = m_user_orders->connect_operation_signal(
    std::bind_front(&MergedBookQuoteListModel::on_user_order_operation, this));
  m_preview_connection = m_preview->connect_update_signal(
    std::bind_front(&MergedBookQuoteListModel::on_preview, this));
}

int MergedBookQuoteListModel::get_size() const {
  auto preview_size = m_previous_preview.has_value() ? 1 : 0;
  return m_book_quotes->get_size() + m_user_orders->get_size() + preview_size;
}

const MergedBookQuoteListModel::Type&
    MergedBookQuoteListModel::get(int index) const {
  if(index < m_book_quotes->get_size()) {
    m_current = m_book_quotes->get(index);
  } else if(index < m_book_quotes->get_size() + m_user_orders->get_size()) {
    m_current = m_user_orders->get(index - m_book_quotes->get_size());
  } else if(m_previous_preview && index == get_size() - 1) {
    m_current = *m_previous_preview;
  } else {
    throw std::out_of_range("The index is out of range.");
  }
  return m_current;
}

connection MergedBookQuoteListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void MergedBookQuoteListModel::transact(
    const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

void MergedBookQuoteListModel::on_book_quote_operation(
    const BookQuoteListModel::Operation& operation) {
  visit(operation,
    [&] (BookQuoteListModel::StartTransaction) {
      m_transaction.start();
    },
    [&] (BookQuoteListModel::EndTransaction) {
      m_transaction.end();
    },
    [&] (const BookQuoteListModel::AddOperation& operation) {
      m_transaction.push(AddOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::PreRemoveOperation& operation) {
      m_transaction.push(PreRemoveOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::RemoveOperation& operation) {
      m_transaction.push(RemoveOperation(operation.m_index));
    },
    [&] (const BookQuoteListModel::MoveOperation& operation) {
      m_transaction.push(
        MoveOperation(operation.m_source, operation.m_destination));
    },
    [&] (const BookQuoteListModel::UpdateOperation& operation) {
      m_transaction.push(UpdateOperation(
        operation.m_index, operation.get_previous(), operation.get_value()));
    });
}

void MergedBookQuoteListModel::on_user_order_operation(
    const BookViewModel::UserOrderListModel::Operation& operation) {
  visit(operation,
    [&] (BookViewModel::UserOrderListModel::StartTransaction) {
      m_transaction.start();
    },
    [&] (BookViewModel::UserOrderListModel::EndTransaction) {
      m_transaction.end();
    },
    [&] (const BookViewModel::UserOrderListModel::AddOperation& operation) {
      m_transaction.push(
        AddOperation(m_book_quotes->get_size() + operation.m_index));
    },
    [&] (const
        BookViewModel::UserOrderListModel::PreRemoveOperation& operation) {
      m_transaction.push(
        PreRemoveOperation(m_book_quotes->get_size() + operation.m_index));
    },
    [&] (const BookViewModel::UserOrderListModel::RemoveOperation& operation) {
      m_transaction.push(
        RemoveOperation(m_book_quotes->get_size() + operation.m_index));
    },
    [&] (const BookViewModel::UserOrderListModel::MoveOperation& operation) {
      m_transaction.push(
        MoveOperation(m_book_quotes->get_size() + operation.m_source,
          m_book_quotes->get_size() + operation.m_destination));
    },
    [&] (const BookViewModel::UserOrderListModel::UpdateOperation& operation) {
      m_transaction.push(UpdateOperation(
        m_book_quotes->get_size() + operation.m_index, operation.get_previous(),
          operation.get_value()));
    });
}

void MergedBookQuoteListModel::on_preview(
    const optional<OrderFields>& preview) {
  if(preview) {
    auto index = m_book_quotes->get_size() + m_user_orders->get_size();
    if(m_previous_preview) {
      auto update = UpdateOperation(index, *m_previous_preview, *preview);
      m_previous_preview = preview;
      m_transaction.push(update);
    } else {
      m_previous_preview = preview;
      m_transaction.push(AddOperation(index));
    }
  } else if(m_previous_preview) {
    auto index = m_book_quotes->get_size() + m_user_orders->get_size();
    m_transaction.transact([&] {
      m_transaction.push(PreRemoveOperation(index));
      m_previous_preview = none;
      m_transaction.push(RemoveOperation(index));
    });
  }
}
