#include "Spire/BookView/TopMpidLevelListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  int find_index(
      const ListModel<TopMpidLevel>& levels, const MarketCode& market) {
    auto i = std::find_if(levels.begin(), levels.end(),
      [&] (const auto& level) {
        return level.m_market == market;
      });
    if(i != levels.end()) {
      return static_cast<int>(std::distance(levels.begin(), i));
    }
    return -1;
  }
}

TopMpidLevelListModel::TopMpidLevelListModel(
    std::shared_ptr<BookQuoteListModel> quotes)
    : m_quotes(std::move(quotes)) {
  for(auto i = 0; i != m_quotes->get_size(); ++i) {
    on_operation(AddOperation(i));
  }
  m_connection = m_quotes->connect_operation_signal(
    std::bind_front(&TopMpidLevelListModel::on_operation, this));
}

int TopMpidLevelListModel::get_size() const {
  return m_top_mpids.get_size();
}

const TopMpidLevelListModel::Type& TopMpidLevelListModel::get(int index) const {
  return m_top_mpids.get(index);
}

connection TopMpidLevelListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_top_mpids.connect_operation_signal(slot);
}

void TopMpidLevelListModel::transact(
    const std::function<void ()>& transaction) {
  return m_top_mpids.transact([&] {
    transaction();
  });
}

void TopMpidLevelListModel::on_operation(
    const BookQuoteListModel::Operation& operation) {
  visit(operation,
    [&] (const BookQuoteListModel::AddOperation& operation) {
      auto quote = m_quotes->get(operation.m_index);
      if(quote.m_isPrimaryMpid) {
        auto top_index = find_index(m_top_mpids, quote.m_market);
        if(top_index != -1) {
          auto& top_mpid = m_top_mpids.get(top_index);
          if(top_mpid.m_index >= operation.m_index) {
            m_top_mpids.set(top_index, TopMpidLevel(
              quote.m_market, operation.m_index, quote.m_quote.m_price));
          }
        } else {
          m_top_mpids.push(TopMpidLevel(
            quote.m_market, operation.m_index, quote.m_quote.m_price));
        }
      }
      for(auto i = 0; i != m_top_mpids.get_size(); ++i) {
        auto& top_mpid = m_top_mpids.get(i);
        if(top_mpid.m_index >= operation.m_index &&
            top_mpid.m_market != quote.m_market) {
          auto update = top_mpid;
          ++update.m_index;
          m_top_mpids.set(i, update);
        }
      }
    },
    [&] (const BookQuoteListModel::PreRemoveOperation& operation) {
      m_removed_quote = m_quotes->get(operation.m_index);
    },
    [&] (const BookQuoteListModel::RemoveOperation& operation) {
      if(m_removed_quote.m_isPrimaryMpid) {
        auto top_index = find_index(m_top_mpids, m_removed_quote.m_market);
        auto& top_mpid = m_top_mpids.get(top_index);
        if(top_mpid.m_index == operation.m_index) {
          for(auto i = operation.m_index; i != m_quotes->get_size(); ++i) {
            if(m_quotes->get(i).m_isPrimaryMpid &&
                m_quotes->get(i).m_market == top_mpid.m_market) {
              auto update = top_mpid;
              update.m_index = i;
              m_top_mpids.set(top_index, update);
              break;
            }
          }
        }
      }
      for(auto i = 0; i != m_top_mpids.get_size(); ++i) {
        auto& top_mpid = m_top_mpids.get(i);
        if(top_mpid.m_index >= operation.m_index &&
            top_mpid.m_market != m_removed_quote.m_market) {
          auto update = top_mpid;
          --update.m_index;
          m_top_mpids.set(i, update);
        }
      }
    });
}
