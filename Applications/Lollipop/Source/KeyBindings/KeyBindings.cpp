#include "Spire/KeyBindings/KeyBindings.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QMessageBox>
#include <QKeyEvent>
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::uuids;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

namespace {
  KeyBindings LoadDefaultKeyBindings() {
    KeyBindings keyBindings;
    keyBindings.SetCancelBinding(QKeySequence(Qt::Key_Escape),
      KeyBindings::CancelBinding::GetCancelBindingFromType(
      KeyBindings::CancelBinding::Type::OLDEST));
    keyBindings.SetCancelBinding(QKeySequence(Qt::SHIFT + Qt::Key_Escape),
      KeyBindings::CancelBinding::GetCancelBindingFromType(
      KeyBindings::CancelBinding::Type::ALL));
    return keyBindings;
  }

  vector<QKeySequence> InitializeAvailableTaskKeyBindings() {
    vector<QKeySequence> keyBindings;
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key(i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key(Qt::CTRL + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key(Qt::ALT + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key(Qt::SHIFT + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key((Qt::CTRL | Qt::ALT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key((Qt::CTRL | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key((Qt::ALT | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_F1; i <= Qt::Key_F12; ++i) {
      QKeySequence key((Qt::CTRL | Qt::ALT | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key(i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key(Qt::CTRL + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key(Qt::ALT + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key(Qt::SHIFT + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key((Qt::CTRL | Qt::ALT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key((Qt::CTRL | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key((Qt::ALT | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    for(int i = Qt::Key_0; i <= Qt::Key_9; ++i) {
      QKeySequence key((Qt::CTRL | Qt::ALT | Qt::SHIFT) + i);
      keyBindings.push_back(key);
    }
    return keyBindings;
  }

  vector<QKeySequence> InitializeAvailableCancelKeyBindings() {
    auto keyBindings = InitializeAvailableTaskKeyBindings();
    keyBindings.push_back(QKeySequence(Qt::Key_Escape));
    keyBindings.push_back(QKeySequence(Qt::CTRL + Qt::Key_Escape));
    keyBindings.push_back(QKeySequence(Qt::ALT + Qt::Key_Escape));
    keyBindings.push_back(QKeySequence(Qt::SHIFT + Qt::Key_Escape));
    keyBindings.push_back(QKeySequence((Qt::CTRL | Qt::ALT) + Qt::Key_Escape));
    keyBindings.push_back(QKeySequence((Qt::CTRL | Qt::SHIFT) +
      Qt::Key_Escape));
    keyBindings.push_back(QKeySequence((Qt::ALT | Qt::SHIFT) + Qt::Key_Escape));
    keyBindings.push_back(QKeySequence((Qt::CTRL | Qt::ALT | Qt::SHIFT) +
      Qt::Key_Escape));
    return keyBindings;
  }
}

KeyModifiers Spire::KeyModifiersFromEvent(const QKeyEvent& event) {
  if(event.modifiers() == Qt::SHIFT) {
    return KeyModifiers::SHIFT;
  } else if(event.modifiers() == Qt::ALT) {
    return KeyModifiers::ALT;
  } else if(event.modifiers() == Qt::CTRL) {
    return KeyModifiers::CONTROL;
  }
  return KeyModifiers::PLAIN;
}

const vector<QKeySequence>& KeyBindings::TaskBinding::GetAvailableKeys() {
  static auto bindings = InitializeAvailableTaskKeyBindings();
  return bindings;
}

KeyBindings::TaskBinding::TaskBinding(string name,
    std::shared_ptr<CanvasNode> node)
    : m_name(std::move(name)),
      m_node(std::move(node)) {}

const vector<QKeySequence>& KeyBindings::CancelBinding::GetAvailableKeys() {
  static auto bindings = InitializeAvailableCancelKeyBindings();
  return bindings;
}

KeyBindings::CancelBinding KeyBindings::CancelBinding::GetCancelBindingFromType(
    Type type) {
  if(type == Type::MOST_RECENT) {
    return CancelBinding("Most recent", Type::MOST_RECENT);
  } else if(type == Type::MOST_RECENT_ASK) {
    return CancelBinding("Most recent ask", Type::MOST_RECENT_ASK);
  } else if(type == Type::MOST_RECENT_BID) {
    return CancelBinding("Most recent bid", Type::MOST_RECENT_BID);
  } else if(type == Type::OLDEST) {
    return CancelBinding("Oldest", Type::OLDEST);
  } else if(type == Type::OLDEST_ASK) {
    return CancelBinding("Oldest ask", Type::OLDEST_ASK);
  } else if(type == Type::OLDEST_BID) {
    return CancelBinding("Oldest bid", Type::OLDEST_BID);
  } else if(type == Type::ALL) {
    return CancelBinding("All", Type::ALL);
  } else if(type == Type::ALL_ASKS) {
    return CancelBinding("All asks", Type::ALL_ASKS);
  } else if(type == Type::ALL_BIDS) {
    return CancelBinding("All bids", Type::ALL_BIDS);
  } else if(type == Type::CLOSEST_ASK) {
    return CancelBinding("Closest ask", Type::CLOSEST_ASK);
  } else if(type == Type::CLOSEST_BID) {
    return CancelBinding("Closest bid", Type::CLOSEST_BID);
  } else if(type == Type::FURTHEST_ASK) {
    return CancelBinding("Furthest ask", Type::FURTHEST_ASK);
  } else if(type == Type::FURTHEST_BID) {
    return CancelBinding("Furthest bid", Type::FURTHEST_BID);
  }
  return CancelBinding("Unknown", Type::NONE);
}

void KeyBindings::CancelBinding::HandleCancel(
    const CancelBinding& cancelBinding,
    Out<vector<std::shared_ptr<Task>>> tasks) {
  if(tasks->empty()) {
    return;
  }
  vector<std::shared_ptr<Task>> tasksToCancel;
  if(cancelBinding.m_type == Type::MOST_RECENT) {
    tasksToCancel.push_back(tasks->back());
    tasks->pop_back();
  } else if(cancelBinding.m_type == Type::MOST_RECENT_ASK) {
    for(auto i = tasks->rbegin(); i != tasks->rend(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::ASK) {
          tasksToCancel.push_back(*i);
          tasks->erase((i + 1).base());
          break;
        }
      }
    }
  } else if(cancelBinding.m_type == Type::MOST_RECENT_BID) {
    for(auto i = tasks->rbegin(); i != tasks->rend(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::BID) {
          tasksToCancel.push_back(*i);
          tasks->erase((i + 1).base());
          break;
        }
      }
    }
  } else if(cancelBinding.m_type == Type::OLDEST) {
    tasksToCancel.push_back(tasks->front());
    tasks->erase(tasks->begin());
  } else if(cancelBinding.m_type == Type::OLDEST_ASK) {
    for(auto i = tasks->begin(); i != tasks->end(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::ASK) {
          tasksToCancel.push_back(*i);
          tasks->erase(i);
          break;
        }
      }
    }
  } else if(cancelBinding.m_type == Type::OLDEST_BID) {
    for(auto i = tasks->begin(); i != tasks->end(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::BID) {
          tasksToCancel.push_back(*i);
          tasks->erase(i);
          break;
        }
      }
    }
  } else if(cancelBinding.m_type == Type::ALL) {
    for(auto i = tasks->begin(); i != tasks->end(); ++i) {
      tasksToCancel.push_back(*i);
    }
    tasks->clear();
  } else if(cancelBinding.m_type == Type::ALL_ASKS) {
    auto i = tasks->begin();
    while(i != tasks->end()) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::ASK) {
          tasksToCancel.push_back(*i);
          i = tasks->erase(i);
          continue;
        }
      }
      ++i;
    }
  } else if(cancelBinding.m_type == Type::ALL_BIDS) {
    auto i = tasks->begin();
    while(i != tasks->end()) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::BID) {
          tasksToCancel.push_back(*i);
          i = tasks->erase(i);
          continue;
        }
      }
      ++i;
    }
  } else if(cancelBinding.m_type == Type::CLOSEST_ASK) {
    auto closestIterator = tasks->rend();
    boost::optional<Money> closestPrice;
    for(auto i = tasks->rbegin(); i != tasks->rend(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::ASK) {
          auto basePriceNode = (*i)->GetNode().FindChild("price");
          if(basePriceNode.is_initialized()) {
            auto priceNode = dynamic_cast<const MoneyNode*>(&*basePriceNode);
            if(priceNode != nullptr && (!closestPrice.is_initialized() ||
                priceNode->GetValue() < *closestPrice)) {
              closestPrice = priceNode->GetValue();
              closestIterator = i;
            }
          }
        }
      }
    }
    if(closestIterator != tasks->rend()) {
      tasksToCancel.push_back(*closestIterator);
      tasks->erase((closestIterator + 1).base());
    }
  } else if(cancelBinding.m_type == Type::CLOSEST_BID) {
    auto closestIterator = tasks->rend();
    boost::optional<Money> closestPrice;
    for(auto i = tasks->rbegin(); i != tasks->rend(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::BID) {
          auto basePriceNode = (*i)->GetNode().FindChild("price");
          if(basePriceNode.is_initialized()) {
            auto priceNode = dynamic_cast<const MoneyNode*>(&*basePriceNode);
            if(priceNode != nullptr && (!closestPrice.is_initialized() ||
                priceNode->GetValue() > *closestPrice)) {
              closestPrice = priceNode->GetValue();
              closestIterator = i;
            }
          }
        }
      }
    }
    if(closestIterator != tasks->rend()) {
      tasksToCancel.push_back(*closestIterator);
      tasks->erase((closestIterator + 1).base());
    }
  } else if(cancelBinding.m_type == Type::FURTHEST_ASK) {
    auto closestIterator = tasks->end();
    boost::optional<Money> closestPrice;
    for(auto i = tasks->begin(); i != tasks->end(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::ASK) {
          auto basePriceNode = (*i)->GetNode().FindChild("price");
          if(basePriceNode.is_initialized()) {
            auto priceNode = dynamic_cast<const MoneyNode*>(&*basePriceNode);
            if(priceNode != nullptr && (!closestPrice.is_initialized() ||
                priceNode->GetValue() >= *closestPrice)) {
              closestPrice = priceNode->GetValue();
              closestIterator = i;
            }
          }
        }
      }
    }
    if(closestIterator != tasks->end()) {
      tasksToCancel.push_back(*closestIterator);
      tasks->erase(closestIterator);
    }
  } else if(cancelBinding.m_type == Type::FURTHEST_BID) {
    auto closestIterator = tasks->end();
    boost::optional<Money> closestPrice;
    for(auto i = tasks->begin(); i != tasks->end(); ++i) {
      auto baseSideNode = (*i)->GetNode().FindChild("side");
      if(baseSideNode.is_initialized()) {
        auto sideNode = dynamic_cast<const SideNode*>(&*baseSideNode);
        if(sideNode != nullptr && sideNode->GetValue() == Side::BID) {
          auto basePriceNode = (*i)->GetNode().FindChild("price");
          if(basePriceNode.is_initialized()) {
            auto priceNode = dynamic_cast<const MoneyNode*>(&*basePriceNode);
            if(priceNode != nullptr && (!closestPrice.is_initialized() ||
                priceNode->GetValue() <= *closestPrice)) {
              closestPrice = priceNode->GetValue();
              closestIterator = i;
            }
          }
        }
      }
    }
    if(closestIterator != tasks->end()) {
      tasksToCancel.push_back(*closestIterator);
      tasks->erase(closestIterator);
    }
  }
  for(auto& i : tasksToCancel) {
    i->Cancel();
  }
}

void KeyBindings::CancelBinding::HandleCancel(
    const CancelBinding& cancelBinding,
    OrderExecutionClient& orderExecutionClient,
    Out<vector<OrderLogModel::OrderEntry>> orders) {
  if(orders->empty()) {
    return;
  }
  vector<std::shared_ptr<Order>> ordersToCancel;
  if(cancelBinding.m_type == Type::MOST_RECENT) {
    ordersToCancel.push_back(orders->back().m_order);
    orders->pop_back();
  } else if(cancelBinding.m_type == Type::MOST_RECENT_ASK) {
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        ordersToCancel.push_back(i->m_order);
        orders->erase((i + 1).base());
        break;
      }
    }
  } else if(cancelBinding.m_type == Type::MOST_RECENT_BID) {
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::BID) {
        ordersToCancel.push_back(i->m_order);
        orders->erase((i + 1).base());
        break;
      }
    }
  } else if(cancelBinding.m_type == Type::OLDEST) {
    ordersToCancel.push_back(orders->front().m_order);
    orders->erase(orders->begin());
  } else if(cancelBinding.m_type == Type::OLDEST_ASK) {
    for(auto i = orders->begin(); i != orders->end(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        ordersToCancel.push_back(i->m_order);
        orders->erase(i);
        break;
      }
    }
  } else if(cancelBinding.m_type == Type::OLDEST_BID) {
    for(auto i = orders->begin(); i != orders->end(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::BID) {
        ordersToCancel.push_back(i->m_order);
        orders->erase(i);
        break;
      }
    }
  } else if(cancelBinding.m_type == Type::ALL) {
    for(auto i = orders->begin(); i != orders->end(); ++i) {
      ordersToCancel.push_back(i->m_order);
    }
    orders->clear();
  } else if(cancelBinding.m_type == Type::ALL_ASKS) {
    auto i = orders->begin();
    while(i != orders->end()) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        ordersToCancel.push_back(i->m_order);
        i = orders->erase(i);
        continue;
      }
      ++i;
    }
  } else if(cancelBinding.m_type == Type::ALL_BIDS) {
    auto i = orders->begin();
    while(i != orders->end()) {
      if(i->m_order->get_info().m_fields.m_side == Side::BID) {
        ordersToCancel.push_back(i->m_order);
        i = orders->erase(i);
        continue;
      }
      ++i;
    }
  } else if(cancelBinding.m_type == Type::CLOSEST_ASK) {
    auto closestIterator = orders->rend();
    boost::optional<Money> closestPrice;
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        if(!closestPrice.is_initialized() ||
            i->m_order->get_info().m_fields.m_price < *closestPrice) {
          closestPrice = i->m_order->get_info().m_fields.m_price;
          closestIterator = i;
        }
      }
    }
    if(closestIterator != orders->rend()) {
      ordersToCancel.push_back(closestIterator->m_order);
      orders->erase((closestIterator + 1).base());
    }
  } else if(cancelBinding.m_type == Type::CLOSEST_BID) {
    auto closestIterator = orders->rend();
    boost::optional<Money> closestPrice;
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::BID) {
        if(!closestPrice.is_initialized() ||
            i->m_order->get_info().m_fields.m_price > *closestPrice) {
          closestPrice = i->m_order->get_info().m_fields.m_price;
          closestIterator = i;
        }
      }
    }
    if(closestIterator != orders->rend()) {
      ordersToCancel.push_back(closestIterator->m_order);
      orders->erase((closestIterator + 1).base());
    }
  } else if(cancelBinding.m_type == Type::FURTHEST_ASK) {
    auto closestIterator = orders->rend();
    boost::optional<Money> closestPrice;
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        if(!closestPrice.is_initialized() ||
            i->m_order->get_info().m_fields.m_price >= *closestPrice) {
          closestPrice = i->m_order->get_info().m_fields.m_price;
          closestIterator = i;
        }
      }
    }
    if(closestIterator != orders->rend()) {
      ordersToCancel.push_back(closestIterator->m_order);
      orders->erase((closestIterator + 1).base());
    }
  } else if(cancelBinding.m_type == Type::FURTHEST_BID) {
    auto closestIterator = orders->rend();
    boost::optional<Money> closestPrice;
    for(auto i = orders->rbegin(); i != orders->rend(); ++i) {
      if(i->m_order->get_info().m_fields.m_side == Side::ASK) {
        if(!closestPrice.is_initialized() ||
            i->m_order->get_info().m_fields.m_price <= *closestPrice) {
          closestPrice = i->m_order->get_info().m_fields.m_price;
          closestIterator = i;
        }
      }
    }
    if(closestIterator != orders->rend()) {
      ordersToCancel.push_back(closestIterator->m_order);
      orders->erase((closestIterator + 1).base());
    }
  }
  for(auto& order : ordersToCancel) {
    orderExecutionClient.cancel(order);
  }
}

KeyBindings::CancelBinding::CancelBinding(string description, Type type)
    : m_description(std::move(description)),
      m_type(std::move(type)) {}

void KeyBindings::Load(Out<UserProfile> userProfile) {
  path keyBindingsFilePath = userProfile->GetProfilePath() / "key_bindings.dat";
  if(!exists(keyBindingsFilePath)) {
    userProfile->SetKeyBindings(LoadDefaultKeyBindings());
    return;
  }
  KeyBindings keyBindings;
  try {
    BasicIStreamReader<ifstream> reader(
      init(keyBindingsFilePath, ios::binary));
    SharedBuffer buffer;
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(keyBindings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load key bindings, using defaults."));
    keyBindings = LoadDefaultKeyBindings();
  }
  userProfile->SetKeyBindings(keyBindings);
}

void KeyBindings::Save(const UserProfile& userProfile) {
  path keyBindingsFilePath = userProfile.GetProfilePath() / "key_bindings.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    sender.shuttle(userProfile.GetKeyBindings());
    BasicOStreamWriter<ofstream> writer(
      init(keyBindingsFilePath, ios::binary));
    writer.write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save key bindings."));
  }
}

boost::optional<const KeyBindings::TaskBinding&>
    KeyBindings::GetTaskFromBinding(Venue venue,
    const QKeySequence& binding) const {
  auto venueBindingsIterator = m_taskBindings.find(venue);
  if(venueBindingsIterator == m_taskBindings.end()) {
    if(!venue) {
      return none;
    }
    return GetTaskFromBinding(Venue(), binding);
  }
  const std::unordered_map<QKeySequence, TaskBinding>& keyBindings =
    venueBindingsIterator->second;
  auto keyBindingsIterator = keyBindings.find(binding);
  if(keyBindingsIterator == keyBindings.end()) {
    if(!venue) {
      return none;
    }
    return GetTaskFromBinding(Venue(), binding);
  }
  auto keyBindingIterator = keyBindings.find(binding);
  if(keyBindingsIterator == keyBindings.end()) {
    return none;
  }
  return keyBindingIterator->second;
}

void KeyBindings::ResetTaskBinding(Venue venue,
    const QKeySequence& binding) {
  m_taskBindings[venue].erase(binding);
}

void KeyBindings::SetTaskBinding(Venue venue, const QKeySequence& binding,
    const TaskBinding& taskBinding) {
  m_taskBindings[venue][binding] = taskBinding;
}

boost::optional<const KeyBindings::CancelBinding&>
    KeyBindings::GetCancelFromBinding(const QKeySequence& binding) const {
  auto keyBindingIterator = m_cancelBindings.find(binding);
  if(keyBindingIterator == m_cancelBindings.end()) {
    return none;
  }
  return keyBindingIterator->second;
}

void KeyBindings::ResetCancelBinding(const QKeySequence& binding) {
  m_cancelBindings.erase(binding);
}

void KeyBindings::SetCancelBinding(const QKeySequence& binding,
    const CancelBinding& cancelBinding) {
  m_cancelBindings[binding] = cancelBinding;
}

Quantity KeyBindings::GetDefaultQuantity(Venue venue) const {
  auto defaultQuantityIterator = m_defaultQuantities.find(venue);
  if(defaultQuantityIterator == m_defaultQuantities.end()) {
    return 100;
  }
  return defaultQuantityIterator->second;
}

void KeyBindings::SetDefaultQuantity(Venue venue, Quantity quantity) {
  m_defaultQuantities[venue] = quantity;
}
