#include "Spire/BookView/PriceLevelModel.hpp"
#include <algorithm>

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
  m_levels.transact([&] {
    transaction();
  });
}

bool PriceLevelModel::insert_level(int index) {
  if(index == 0) {
    m_levels.insert(0, 0);
    return true;
  }
  auto preceding_level = m_levels.get(index - 1);
  if(m_prices->get(index - 1) == m_prices->get(index) ||
      preceding_level == m_max_level->get()) {
    m_levels.insert(preceding_level, index);
    return false;
  }
  m_levels.insert(preceding_level + 1, index);
  return true;
}

void PriceLevelModel::shift_levels_after(int index) {
  if(index == m_prices->get_size() - 1 ||
      m_prices->get(index + 1) == m_prices->get(index)) {
    return;
  }
  for(auto i = index + 1; i != m_levels.get_size(); ++i) {
    auto level = m_levels.get(i);
    if(level == m_max_level->get()) {
      break;
    }
    m_levels.set(i, level + 1);
  }
}

void PriceLevelModel::collapse_levels_from(int index, int level) {
  if(index != 0 && m_levels.get(index - 1) == level) {
    return;
  }
  if(index == m_prices->get_size()) {
    return;
  }
  auto following_level = m_levels.get(index);
  if(following_level == level) {
    return;
  }
  auto following_price = m_prices->get(index);
  for(auto i = index; i != m_levels.get_size(); ++i) {
    auto price = m_prices->get(i);
    if(price != following_price) {
      ++following_level;
      following_price = price;
      if(following_level == m_max_level->get() + 1) {
        break;
      }
    }
    m_levels.set(i, m_levels.get(i) - 1);
  }
}

void PriceLevelModel::update_levels(int index) {
  m_levels.transact([&] {
    for(auto i = index; i != m_levels.get_size(); ++i) {
      auto level = [&] {
        if(i == 0) {
          return 0;
        } else if(m_prices->get(i - 1) == m_prices->get(i)) {
          return m_levels.get(i - 1);
        }
        return std::min(m_levels.get(i - 1) + 1, m_max_level->get());
      }();
      if(m_levels.get(i) != level) {
        m_levels.set(i, level);
      }
    }
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
      m_levels.transact([&] {
        if(insert_level(operation.m_index)) {
          shift_levels_after(operation.m_index);
        }
      });
    },
    [&] (const PriceListModel::RemoveOperation& operation) {
      auto level = m_levels.get(operation.m_index);
      m_levels.transact([&] {
        m_levels.remove(operation.m_index);
        collapse_levels_from(operation.m_index, level);
      });
    },
    [&] (const PriceListModel::MoveOperation& operation) {
      m_levels.move(operation.m_source, operation.m_destination);
    },
    [&] (const PriceListModel::UpdateOperation& operation) {
      if(operation.get_previous() == operation.get_value()) {
        return;
      }
      update_levels(operation.m_index);
    });
}

void PriceLevelModel::on_max_level(int max_level) {
  m_levels.transact([&] {
    auto previous_price = Money();
    auto current_level = 0;
    for(auto i = 0; i < m_prices->get_size(); ++i) {
      auto price = m_prices->get(i);
      if(i > 0 && price != previous_price && current_level < max_level) {
        ++current_level;
      }
      previous_price = price;
      if(m_levels.get(i) != current_level) {
        m_levels.set(i, current_level);
      }
    }
  });
}
