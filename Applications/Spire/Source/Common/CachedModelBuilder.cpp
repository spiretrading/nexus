#include <utility>
#include <QObject>
#include "Spire/Spire/CachedModelBuilder.hpp"

using namespace Spire;
using namespace Spire::Details;

ExpiryTimer& ExpiryTimer::get_instance() {
  static auto instance = ExpiryTimer();
  return instance;
}

std::int64_t ExpiryTimer::add(std::function<void ()> callback) {
  auto id = ++m_next_id;
  m_callbacks.emplace(id, std::move(callback));
  return id;
}

void ExpiryTimer::remove(std::int64_t id) {
  m_callbacks.erase(id);
}

ExpiryTimer::ExpiryTimer()
    : m_next_id(0) {
  QObject::connect(&m_timer, &QTimer::timeout, [this] {
    for(auto& callback : m_callbacks) {
      callback.second();
    }
  });
  m_timer.start(EXPIRY_INTERVAL);
}
