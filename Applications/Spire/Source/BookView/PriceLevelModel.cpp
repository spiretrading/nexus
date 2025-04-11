#include "Spire/BookView/PriceLevelModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

PriceLevelModel::PriceLevelModel(std::shared_ptr<PriceListModel> prices,
    std::shared_ptr<MaxLevelModel> max_level)
    : m_prices(std::move(prices)),
      m_max_level(std::move(max_level)) {
  for(auto i = 0; i != m_prices->get_size(); ++i) {
    on_price_operation(PriceListModel::AddOperation(i));
  }
  m_price_connection = m_prices->connect_operation_signal(
    std::bind_front(&PriceLevelModel::on_price_operation, this));
  m_max_level_connection = m_max_level->connect_update_signal(
    std::bind_front(&PriceLevelModel::on_max_level, this));
}

int PriceLevelModel::get_size() const {
  return m_levels.get_size();
}

const PriceLevelModel::Type& PriceLevelModel::get(int index) const {
  return m_levels.get(index);
}

connection PriceLevelModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_levels.connect_operation_signal(slot);
}

void PriceLevelModel::transact(const std::function<void ()>& transaction) {
  return m_levels.transact([&] {
    transaction();
  });
}

void PriceLevelModel::on_price_operation(
    const PriceListModel::Operation& operation) {
  visit(operation,
    [&] (const PriceListModel::AddOperation& operation) {
      if(m_levels.get_size() == 0) {
        m_levels.push(0);
        return;
      }
      auto price = m_prices->get(operation.m_index);
      if(operation.m_index == 0) {
        m_levels.insert(0, 0);
      } else {
        auto preceding_price = m_prices->get(operation.m_index - 1);
        auto preceding_level = m_levels.get(operation.m_index - 1);
        if(preceding_price == price || preceding_level == m_max_level->get()) {
          m_levels.insert(preceding_level, operation.m_index);
          return;
        } else {
          m_levels.insert(preceding_level + 1, operation.m_index);
        }
      }
      if(operation.m_index != m_prices->get_size() - 1) {
        auto following_price = m_prices->get(operation.m_index + 1);
        if(following_price != price) {
          for(auto i = operation.m_index + 1; i != m_levels.get_size(); ++i) {
            auto level = m_levels.get(i);
            if(level == m_max_level->get()) {
              break;
            }
            m_levels.set(i, level + 1);
          }
        }
      }
    },
    [&] (const PriceListModel::PreRemoveOperation& operation) {
      m_removed_price = m_prices->get(operation.m_index);
    },
    [&] (const PriceListModel::RemoveOperation& operation) {
      auto level = m_levels.get(operation.m_index);
      m_levels.remove(operation.m_index);
      if(operation.m_index != 0) {
        auto preceding_level = m_levels.get(operation.m_index - 1);
        if(preceding_level == level) {
          return;
        }
      }
      if(operation.m_index != m_prices->get_size()) {
        auto following_level = m_levels.get(operation.m_index);
        if(following_level == level) {
          return;
        }
        auto following_price = m_prices->get(operation.m_index);
        for(auto i = operation.m_index; i != m_levels.get_size(); ++i) {
          auto price = m_prices->get(i);
          if(price != following_price) {
            ++following_level;
            if(following_level == m_max_level->get()) {
              break;
            }
          }
          auto level = m_levels.get(i);
          m_levels.set(i, level - 1);
        }
      }
    });
}

void PriceLevelModel::on_max_level(int max_level) {
}
