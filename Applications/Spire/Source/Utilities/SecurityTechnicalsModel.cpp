#include "Spire/Utilities/SecurityTechnicalsModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::ChartingService;
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;
using namespace Nexus::TechnicalAnalysis;
using namespace Spire;

namespace {
  const auto EXPIRY_INTERVAL = 3000;

  std::unordered_map<Security, std::weak_ptr<SecurityTechnicalsModel>>
    existingModels;
  std::unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    pendingExpiryModels;
  std::unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    expiringModels;
  std::unique_ptr<QTimer> expiryTimer;

  void ModelDeleter(const Security& security, SecurityTechnicalsModel* model) {
    pendingExpiryModels.insert(std::pair(
      security, std::unique_ptr<SecurityTechnicalsModel>{model}));
  }

  void OnExpiryTimer() {
    expiringModels = std::move(pendingExpiryModels);
    pendingExpiryModels.clear();
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
  if(!expiryTimer) {
    expiryTimer = std::make_unique<QTimer>();
    expiryTimer->start(EXPIRY_INTERVAL);
    QObject::connect(expiryTimer.get(), &QTimer::timeout, &OnExpiryTimer);
  }
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
    : m_userProfile(userProfile.Get()),
      m_volume(0) {
  if(security == Security()) {
    return;
  }
  SecurityMarketDataQuery timeAndSaleQuery;
  timeAndSaleQuery.SetIndex(security);
  timeAndSaleQuery.SetRange(Beam::Queries::Range::RealTime());
  timeAndSaleQuery.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    timeAndSaleQuery, m_slotHandler.GetSlot<TimeAndSale>(std::bind(
    &SecurityTechnicalsModel::OnTimeAndSale, this, std::placeholders::_1)));
  Spawn(
    [=, userProfile = m_userProfile,
        loadTechnicalsFlag = m_loadTechnicalsFlag] {
      auto securityTechnicals = userProfile->GetServiceClients().
        GetMarketDataClient().LoadSecurityTechnicals(security);
      With(*loadTechnicalsFlag,
        [=] (bool loadTechnicalsFlag) {
          if(!loadTechnicalsFlag) {
            return;
          }
          m_slotHandler.Push(
            [=] {
              if(securityTechnicals.m_open != Money::ZERO) {
                m_open = securityTechnicals.m_open;
                m_openSignal(m_open);
              }
              if(securityTechnicals.m_close != Money::ZERO) {
                m_close = securityTechnicals.m_close;
                m_closeSignal(m_close);
              }
              if(securityTechnicals.m_high != Money::ZERO) {
                m_high = securityTechnicals.m_high;
                m_highSignal(m_high);
              }
              if(securityTechnicals.m_low != Money::ZERO) {
                m_low = securityTechnicals.m_low;
                m_lowSignal(m_low);
              }
              if(securityTechnicals.m_volume != 0) {
                m_volume = securityTechnicals.m_volume;
                m_volumeSignal(m_volume);
              }
            });
        });
    });
  connect(&m_updateTimer, &QTimer::timeout, this,
    &SecurityTechnicalsModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
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

void SecurityTechnicalsModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
