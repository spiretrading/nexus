#include "Spire/BookView/DefaultQuantityModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

DefaultQuantityModel::DefaultQuantityModel(
    Ref<UserProfile> user_profile, Security security, Side side)
    : m_user_profile(user_profile.get()),
      m_security(std::move(security)),
      m_side(side) {
  const auto UPDATE_INTERVAL_MS = 300;
  m_update_timer.setInterval(UPDATE_INTERVAL_MS);
  on_update();
  m_update_timer.start();
}


const DefaultQuantityModel::Type& DefaultQuantityModel::get() const {
  return m_current.get();
}

connection DefaultQuantityModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}

void DefaultQuantityModel::on_update() {
  auto quantity =
    get_default_order_quantity(*m_user_profile, m_security, m_side);
  if(quantity != m_current.get()) {
    m_current.set(quantity);
  }
}
