#include "Spire/Utilities/SecurityTechnicalsModel.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"
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
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
  const auto EXPIRY_INTERVAL = 3000;

  unordered_map<Security, std::weak_ptr<SecurityTechnicalsModel>>
    existingModels;
  unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    pendingExpiryModels;
  unordered_map<Security, std::unique_ptr<SecurityTechnicalsModel>>
    expiringModels;
  unique_ptr<QTimer> expiryTimer;

  void ModelDeleter(const Security& security, SecurityTechnicalsModel* model) {
    pendingExpiryModels.insert(std::make_pair(security,
      std::unique_ptr<SecurityTechnicalsModel>{model}));
  }

  void OnExpiryTimer() {
    expiringModels = std::move(pendingExpiryModels);
    pendingExpiryModels.clear();
  }

  std::shared_ptr<SecurityTechnicalsModel> FindModel(const Security& security) {
    std::shared_ptr<SecurityTechnicalsModel> model;
    auto existingModelIterator = existingModels.find(security);
    if(existingModelIterator != existingModels.end()) {
      model = existingModelIterator->second.lock();
      if(model == nullptr) {
        existingModels.erase(security);
      }
    }
    if(model == nullptr) {
      auto pendingExpiryModelIterator = pendingExpiryModels.find(security);
      if(pendingExpiryModelIterator != pendingExpiryModels.end()) {
        model = std::move(pendingExpiryModelIterator->second);
        pendingExpiryModels.erase(security);
        existingModels.insert(std::make_pair(security, model));
      }
    }
    if(model == nullptr) {
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
  if(expiryTimer == nullptr) {
    expiryTimer = make_unique<QTimer>();
    expiryTimer->start(EXPIRY_INTERVAL);
    QObject::connect(expiryTimer.get(), &QTimer::timeout, &OnExpiryTimer);
  }
  auto model = FindModel(security);
  if(model == nullptr) {
    model.reset(new SecurityTechnicalsModel{Ref(userProfile), security},
      std::bind(&ModelDeleter, security, std::placeholders::_1));
    existingModels.insert(std::make_pair(security, model));
  }
  return model;
}

SecurityTechnicalsModel::~SecurityTechnicalsModel() {
  *m_loadTechnicalsFlag = false;
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
      m_loadTechnicalsFlag(std::make_shared<Sync<bool>>(true)),
      m_volume(0) {
  if(security == Security()) {
    return;
  }
  QueryDailyHigh(m_userProfile->GetServiceClients().GetChartingClient(),
    security, m_userProfile->GetServiceClients().GetTimeClient().GetTime(),
    pos_infin, m_userProfile->GetMarketDatabase(),
    m_userProfile->GetTimeZoneDatabase(),
    m_slotHandler.GetSlot<Nexus::Queries::QueryVariant>(std::bind(
    &SecurityTechnicalsModel::OnHighUpdate, this, std::placeholders::_1)));
  QueryDailyLow(m_userProfile->GetServiceClients().GetChartingClient(),
    security, m_userProfile->GetServiceClients().GetTimeClient().GetTime(),
    pos_infin, m_userProfile->GetMarketDatabase(),
    m_userProfile->GetTimeZoneDatabase(),
    m_slotHandler.GetSlot<Nexus::Queries::QueryVariant>(std::bind(
    &SecurityTechnicalsModel::OnLowUpdate, this, std::placeholders::_1)));
  QueryDailyVolume(m_userProfile->GetServiceClients().GetChartingClient(),
    security, m_userProfile->GetServiceClients().GetTimeClient().GetTime(),
    pos_infin, m_userProfile->GetMarketDatabase(),
    m_userProfile->GetTimeZoneDatabase(),
    m_slotHandler.GetSlot<Nexus::Queries::QueryVariant>(std::bind(
    &SecurityTechnicalsModel::OnVolumeUpdate, this, std::placeholders::_1)));
  QueryOpen(userProfile->GetServiceClients().GetMarketDataClient(),
    security, userProfile->GetServiceClients().GetTimeClient().GetTime(),
    userProfile->GetMarketDatabase(), userProfile->GetTimeZoneDatabase(), "",
    m_slotHandler.GetSlot<TimeAndSale>(std::bind(
    &SecurityTechnicalsModel::OnOpenUpdate, this, std::placeholders::_1)));
  Spawn(
    [=, userProfile = m_userProfile,
        loadTechnicalsFlag = m_loadTechnicalsFlag] {
      auto close = LoadPreviousClose(
        userProfile->GetServiceClients().GetMarketDataClient(), security,
        userProfile->GetServiceClients().GetTimeClient().GetTime(),
        userProfile->GetMarketDatabase(), userProfile->GetTimeZoneDatabase(),
        "");
      With(*loadTechnicalsFlag,
        [=] (bool loadTechnicalsFlag) {
          if(!loadTechnicalsFlag) {
            return;
          }
          m_slotHandler.Push(
            [=] {
              if(close.is_initialized()) {
                m_close = close->m_price;
                m_closeSignal(m_close);
              }
            });
        });
    });
  connect(&m_updateTimer, &QTimer::timeout, this,
    &SecurityTechnicalsModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
}

void SecurityTechnicalsModel::OnOpenUpdate(const TimeAndSale& timeAndSale) {
  m_open = timeAndSale.m_price;
  m_openSignal(m_open);
}

void SecurityTechnicalsModel::OnHighUpdate(
    const Nexus::Queries::QueryVariant& high) {
  m_high = boost::get<Money>(high);
  m_highSignal(m_high);
}

void SecurityTechnicalsModel::OnLowUpdate(
    const Nexus::Queries::QueryVariant& low) {
  m_low = boost::get<Money>(low);
  m_lowSignal(m_low);
}

void SecurityTechnicalsModel::OnVolumeUpdate(
    const Nexus::Queries::QueryVariant& volume) {
  m_volume = boost::get<Quantity>(volume);
  m_volumeSignal(m_volume);
}

void SecurityTechnicalsModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
