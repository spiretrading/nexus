#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  Side get_side(const CanvasNode& node) {
    if(auto base_side_node = node.FindChild("side")) {
      if(auto side_node = dynamic_cast<const SideNode*>(&*base_side_node)) {
        return side_node->GetValue();
      }
    }
    return Side::NONE;
  }

  Side get_side(CancelKeyBindingsModel::Operation operation) {
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
        operation == CancelKeyBindingsModel::Operation::OLDEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::ALL_ASKS ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK) {
      return Side::ASK;
    } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
        operation == CancelKeyBindingsModel::Operation::OLDEST_BID ||
        operation == CancelKeyBindingsModel::Operation::ALL_BIDS ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
        operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
      return Side::BID;
    }
    return Side::NONE;
  }

  optional<Money> get_price(const CanvasNode& node) {
    if(auto base_price_node = node.FindChild("price")) {
      if(auto price_node = dynamic_cast<const MoneyNode*>(&*base_price_node)) {
        return price_node->GetValue();
      }
    }
    return none;
  }

  template<typename T>
  auto to_base(const T& i) {
    return i;
  }

  template<typename T>
  auto to_base(const std::reverse_iterator<T>& i) {
    return (i + 1).base();
  }
}

CancelKeyBindingsModel::CancelKeyBindingsModel() {
  for(auto i = 0; i < OPERATION_COUNT; ++i) {
    auto operation = static_cast<Operation>(i);
    m_bindings[i] = make_validated_value_model(
      std::bind_front(&CancelKeyBindingsModel::on_validate, this, operation),
      std::make_shared<LocalKeySequenceValueModel>());
    m_bindings[i]->connect_update_signal(
      std::bind_front(&CancelKeyBindingsModel::on_update, this, operation));
  }
}

std::shared_ptr<KeySequenceValueModel>
    CancelKeyBindingsModel::get_binding(Operation operation) const {
  return m_bindings[static_cast<int>(operation)];
}

optional<CancelKeyBindingsModel::Operation>
    CancelKeyBindingsModel::find_operation(const QKeySequence& sequence) const {
  if(auto i = m_bindings_map.find(sequence); i != m_bindings_map.end()) {
    return i->second;
  }
  return none;
}

QValidator::State CancelKeyBindingsModel::on_validate(Operation operation,
    const QKeySequence& sequence) {
  if(sequence.count() == 0) {
    return QValidator::Intermediate;
  } else if(sequence.count() > 1) {
    return QValidator::Invalid;
  }
  auto key = sequence[0];
  auto modifier = key & Qt::KeyboardModifierMask;
  key -= modifier;
  if(((modifier == Qt::NoModifier || modifier & Qt::ControlModifier ||
    modifier & Qt::ShiftModifier || modifier & Qt::AltModifier) &&
      ((key >= Qt::Key_F1 && key <= Qt::Key_F12) ||
        (key >= Qt::Key_0 && key <= Qt::Key_9) || key == Qt::Key_Escape))) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

void CancelKeyBindingsModel::on_update(Operation operation,
    const QKeySequence& sequence) {
  auto update = [&] {
    m_bindings_map.erase(m_previous_bindings[static_cast<int>(operation)]);
    if(!sequence.isEmpty()) {
      m_bindings_map[sequence] = operation;
    }
    m_previous_bindings[static_cast<int>(operation)] = sequence;
  };
  if(auto search = find_operation(sequence); search) {
    if(*search != operation) {
      m_bindings[static_cast<int>(*search)]->set(QKeySequence());
      update();
    }
  } else {
    update();
  }
}

void Spire::execute(CancelKeyBindingsModel::Operation operation,
    Out<std::vector<std::shared_ptr<Task>>> tasks) {
  if(tasks->empty()) {
    return;
  }
  auto tasks_to_cancel = std::vector<std::shared_ptr<Task>>();
  auto expected_side = get_side(operation);
  auto direction = get_direction(expected_side);
  if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT) {
    tasks_to_cancel.push_back(tasks->back());
    tasks->pop_back();
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST) {
    tasks_to_cancel.push_back(tasks->front());
    tasks->erase(tasks->begin());
  } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
      operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_BID) {
    auto cancel = [&] (auto begin, auto end) {
      for(auto i = begin; i != end; ++i) {
        if(get_side((*i)->GetNode()) == expected_side) {
          tasks_to_cancel.push_back(*i);
          tasks->erase(to_base(i));
          break;
        }
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
        operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID) {
      cancel(tasks->rbegin(), tasks->rend());
    } else {
      cancel(tasks->begin(), tasks->end());
    }
  } else if(operation == CancelKeyBindingsModel::Operation::ALL) {
    tasks_to_cancel = std::move(*tasks);
    tasks->clear();
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_ASKS ||
      operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
    std::erase_if(*tasks, [&] (const auto& task) {
      if(get_side(task->GetNode()) != expected_side) {
        return false;
      }
      tasks_to_cancel.push_back(task);
      return true;
    });
  } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
    auto cancel = [&] (auto begin, auto end, auto flip) {
      auto closest_iterator = end;
      auto closest_price = optional<Money>();
      for(auto i = begin; i != end; ++i) {
        if(get_side((*i)->GetNode()) == expected_side) {
          if(auto price = get_price((*i)->GetNode())) {
            if(closest_price && (*price < direction * *closest_price) == flip) {
              closest_price = *price;
              closest_iterator = i;
            }
          }
        }
      }
      if(closest_iterator != end) {
        tasks_to_cancel.push_back(*closest_iterator);
        tasks->erase(to_base(closest_iterator));
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_BID) {
      cancel(tasks->rbegin(), tasks->rend(), true);
    } else {
      cancel(tasks->begin(), tasks->end(), false);
    }
  }
  for(auto& i : tasks_to_cancel) {
    i->Cancel();
  }
}

void Spire::execute(
    CancelKeyBindingsModel::Operation operation, OrderExecutionClient& client,
    Out<std::vector<OrderLogModel::OrderEntry>> entries) {
  if(entries->empty()) {
    return;
  }
  auto orders_to_cancel = std::vector<std::shared_ptr<const Order>>();
  auto expected_side = get_side(operation);
  auto direction = get_direction(expected_side);
  if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT) {
    orders_to_cancel.push_back(entries->back().m_order);
    entries->pop_back();
  } else if(operation == CancelKeyBindingsModel::Operation::OLDEST) {
    orders_to_cancel.push_back(entries->front().m_order);
    entries->erase(entries->begin());
  } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
      operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::OLDEST_BID) {
    auto cancel = [&] (auto begin, auto end) {
      for(auto i = begin; i != end; ++i) {
        if(i->m_order->get_info().m_fields.m_side == expected_side) {
          orders_to_cancel.push_back(i->m_order);
          entries->erase(to_base(i));
          break;
        }
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK ||
        operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID) {
      cancel(entries->rbegin(), entries->rend());
    } else {
      cancel(entries->begin(), entries->end());
    }
  } else if(operation == CancelKeyBindingsModel::Operation::ALL) {
    for(auto& entry : *entries) {
      orders_to_cancel.push_back(entry.m_order);
    }
    entries->clear();
  } else if(operation == CancelKeyBindingsModel::Operation::ALL_ASKS ||
      operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
    std::erase_if(*entries, [&] (const auto& entry) {
      if(entry.m_order->get_info().m_fields.m_side != expected_side) {
        return false;
      }
      orders_to_cancel.push_back(entry.m_order);
      return true;
    });
  } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::CLOSEST_BID ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK ||
      operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
    auto cancel = [&] (auto begin, auto end, auto flip) {
      auto closest_iterator = end;
      auto closest_price = optional<Money>();
      for(auto i = begin; i != end; ++i) {
        if(i->m_order->get_info().m_fields.m_side == expected_side) {
          auto price = i->m_order->get_info().m_fields.m_price;
          if(price != Money::ZERO) {
            if(closest_price && (price < direction * *closest_price) == flip) {
              closest_price = price;
              closest_iterator = i;
            }
          }
        }
      }
      if(closest_iterator != end) {
        orders_to_cancel.push_back(closest_iterator->m_order);
        entries->erase(to_base(closest_iterator));
      }
    };
    if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK ||
        operation == CancelKeyBindingsModel::Operation::CLOSEST_BID) {
      cancel(entries->rbegin(), entries->rend(), true);
    } else {
      cancel(entries->begin(), entries->end(), false);
    }
  }
  for(auto& order : orders_to_cancel) {
    client.cancel(*order);
  }
}
