#include "Spire/Utilities/SecurityTechnicalsModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto EXPIRY_INTERVAL = 3000;

  std::unordered_map<Security, std::weak_ptr<SecurityTechnicalsModel>>
    existingModels;
  std::unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    pendingExpiryModels;
  std::unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    expiringModels;

  void ModelDeleter(const Security& security, SecurityTechnicalsModel* model) {
    pendingExpiryModels.insert(
      std::pair(security, std::unique_ptr<SecurityTechnicalsModel>(model)));
  }

  std::shared_ptr<SecurityTechnicalsModel> FindModel(const Security& security) {
    auto model = std::shared_ptr<SecurityTechnicalsModel>();
    auto existingModelIterator = existingModels.find(security);
    if(existingModelIterator != existingModels.end()) {
      model = existingModelIterator->second.lock();
      if(!model) {
        existingModels.erase(security);
      }
    }
    if(!model) {
      auto pendingExpiryModelIterator = pendingExpiryModels.find(security);
      if(pendingExpiryModelIterator != pendingExpiryModels.end()) {
        model = std::move(pendingExpiryModelIterator->second);
        pendingExpiryModels.erase(security);
        existingModels.insert(std::make_pair(security, model));
      }
    }
    if(!model) {
      auto expiringModelIterator = expiringModels.find(security);
      if(expiringModelIterator != expiringModels.end()) {
        model = std::move(expiringModelIterator->second);
        expiringModels.erase(security);
        existingModels.insert(std::make_pair(security, model));
      }
    }
    return model;
  }
}

std::shared_ptr<SecurityTechnicalsModel> SecurityTechnicalsModel::GetModel(
    Ref<UserProfile> userProfile, const Security& security) {
  static auto expiryTimer = [] {
    auto expiryTimer = std::make_unique<QTimer>();
    expiryTimer->start(EXPIRY_INTERVAL);
    QObject::connect(expiryTimer.get(), &QTimer::timeout, [] {
      expiringModels = std::move(pendingExpiryModels);
      pendingExpiryModels.clear();
    });
    return expiryTimer;
  }();
  auto model = FindModel(security);
  if(!model) {
    model.reset(new SecurityTechnicalsModel(Ref(userProfile), security),
      std::bind_front(&ModelDeleter, security));
    existingModels.insert(std::make_pair(security, model));
  }
  return model;
}

connection SecurityTechnicalsModel::ConnectOpenSignal(
    const OpenSignal::slot_type& slot) const {
  if(m_open != Money::ZERO) {
    slot(m_open);
  }
  return m_openSignal.connect(slot);
}

connection SecurityTechnicalsModel::ConnectCloseSignal(
    const CloseSignal::slot_type& slot) const {
  if(m_close != Money::ZERO) {
    slot(m_close);
  }
  return m_closeSignal.connect(slot);
}

connection SecurityTechnicalsModel::ConnectHighSignal(
    const HighSignal::slot_type& slot) const {
  if(m_high != Money::ZERO) {
    slot(m_high);
  }
  return m_highSignal.connect(slot);
}

connection SecurityTechnicalsModel::ConnectLowSignal(
    const LowSignal::slot_type& slot) const {
  if(m_low != Money::ZERO) {
    slot(m_low);
  }
  return m_lowSignal.connect(slot);
}

connection SecurityTechnicalsModel::ConnectVolumeSignal(
    const VolumeSignal::slot_type& slot) const {
  slot(m_volume);
  return m_volumeSignal.connect(slot);
}

SecurityTechnicalsModel::SecurityTechnicalsModel(
    Ref<UserProfile> userProfile, const Security& security)
    : m_userProfile(userProfile.get()),
      m_volume(0) {
  if(security == Security()) {
    return;
  }
  auto timeAndSaleQuery = make_real_time_query(security);
  timeAndSaleQuery.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetClients().get_market_data_client().query(
    timeAndSaleQuery, m_eventHandler.get_slot<TimeAndSale>(
      std::bind_front(&SecurityTechnicalsModel::OnTimeAndSale, this)));
  m_loadPromise = std::make_shared<QtPromise<void>>(QtPromise([=] {
    return userProfile->GetClients().get_market_data_client().load_technicals(
      security);
  }, LaunchPolicy::ASYNC).then([=] (const SecurityTechnicals& technicals) {
    if(technicals.m_open != Money::ZERO) {
      m_open = technicals.m_open;
      m_openSignal(m_open);
    }
    if(technicals.m_close != Money::ZERO) {
      m_close = technicals.m_close;
      m_closeSignal(m_close);
    }
    if(technicals.m_high != Money::ZERO) {
      m_high = technicals.m_high;
      m_highSignal(m_high);
    }
    if(technicals.m_low != Money::ZERO) {
      m_low = technicals.m_low;
      m_lowSignal(m_low);
    }
    if(technicals.m_volume != 0) {
      m_volume = technicals.m_volume;
      m_volumeSignal(m_volume);
    }
    m_loadPromise = nullptr;
  }));
}

void SecurityTechnicalsModel::OnTimeAndSale(const TimeAndSale& timeAndSale) {
  m_volume += timeAndSale.m_size;
  m_volumeSignal(m_volume);
  if(m_open == Money::ZERO) {
    m_open = timeAndSale.m_price;
    m_openSignal(m_open);
  }
  if(timeAndSale.m_price > m_high) {
    m_high = timeAndSale.m_price;
    m_highSignal(m_high);
  }
  if(timeAndSale.m_price < m_low || m_low == Money::ZERO) {
    m_low = timeAndSale.m_price;
    m_lowSignal(m_low);
  }
}
