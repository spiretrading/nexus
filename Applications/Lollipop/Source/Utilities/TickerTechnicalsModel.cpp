#include "Spire/Utilities/TickerTechnicalsModel.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto EXPIRY_INTERVAL = 3000;

  std::unordered_map<Ticker, std::weak_ptr<TickerTechnicalsModel>>
    existingModels;
  std::unordered_map<Ticker, std::unique_ptr<TickerTechnicalsModel>>
    pendingExpiryModels;
  std::unordered_map<Ticker, std::unique_ptr<TickerTechnicalsModel>>
    expiringModels;

  void ModelDeleter(const Ticker& ticker, TickerTechnicalsModel* model) {
    pendingExpiryModels.insert(
      std::pair(ticker, std::unique_ptr<TickerTechnicalsModel>(model)));
  }

  std::shared_ptr<TickerTechnicalsModel> FindModel(const Ticker& ticker) {
    auto model = std::shared_ptr<TickerTechnicalsModel>();
    auto existingModelIterator = existingModels.find(ticker);
    if(existingModelIterator != existingModels.end()) {
      model = existingModelIterator->second.lock();
      if(!model) {
        existingModels.erase(ticker);
      }
    }
    if(!model) {
      auto pendingExpiryModelIterator = pendingExpiryModels.find(ticker);
      if(pendingExpiryModelIterator != pendingExpiryModels.end()) {
        model = std::move(pendingExpiryModelIterator->second);
        pendingExpiryModels.erase(ticker);
        existingModels.insert(std::make_pair(ticker, model));
      }
    }
    if(!model) {
      auto expiringModelIterator = expiringModels.find(ticker);
      if(expiringModelIterator != expiringModels.end()) {
        model = std::move(expiringModelIterator->second);
        expiringModels.erase(ticker);
        existingModels.insert(std::make_pair(ticker, model));
      }
    }
    return model;
  }
}

std::shared_ptr<TickerTechnicalsModel> TickerTechnicalsModel::GetModel(
    Ref<UserProfile> userProfile, const Ticker& ticker) {
  static auto expiryTimer = [] {
    auto expiryTimer = std::make_unique<QTimer>();
    expiryTimer->start(EXPIRY_INTERVAL);
    QObject::connect(expiryTimer.get(), &QTimer::timeout, [] {
      expiringModels = std::move(pendingExpiryModels);
      pendingExpiryModels.clear();
    });
    return expiryTimer;
  }();
  auto model = FindModel(ticker);
  if(!model) {
    model.reset(new TickerTechnicalsModel(Ref(userProfile), ticker),
      std::bind_front(&ModelDeleter, ticker));
    existingModels.insert(std::make_pair(ticker, model));
  }
  return model;
}

connection TickerTechnicalsModel::ConnectOpenSignal(
    const OpenSignal::slot_type& slot) const {
  if(m_open != Money::ZERO) {
    slot(m_open);
  }
  return m_openSignal.connect(slot);
}

connection TickerTechnicalsModel::ConnectCloseSignal(
    const CloseSignal::slot_type& slot) const {
  if(m_close != Money::ZERO) {
    slot(m_close);
  }
  return m_closeSignal.connect(slot);
}

connection TickerTechnicalsModel::ConnectHighSignal(
    const HighSignal::slot_type& slot) const {
  if(m_high != Money::ZERO) {
    slot(m_high);
  }
  return m_highSignal.connect(slot);
}

connection TickerTechnicalsModel::ConnectLowSignal(
    const LowSignal::slot_type& slot) const {
  if(m_low != Money::ZERO) {
    slot(m_low);
  }
  return m_lowSignal.connect(slot);
}

connection TickerTechnicalsModel::ConnectVolumeSignal(
    const VolumeSignal::slot_type& slot) const {
  slot(m_volume);
  return m_volumeSignal.connect(slot);
}

TickerTechnicalsModel::TickerTechnicalsModel(
    Ref<UserProfile> userProfile, const Ticker& ticker)
    : m_userProfile(userProfile.get()),
      m_volume(0) {
  if(ticker == Ticker()) {
    return;
  }
  auto timeAndSaleQuery = make_real_time_query(ticker);
  timeAndSaleQuery.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetClients().get_market_data_client().query(
    timeAndSaleQuery, m_eventHandler.get_slot<TimeAndSale>(
      std::bind_front(&TickerTechnicalsModel::OnTimeAndSale, this)));
  m_loadPromise = std::make_shared<QtPromise<void>>(QtPromise([=] {
    return userProfile->GetClients().get_market_data_client().
      load_session_candlestick(ticker);
  }, LaunchPolicy::ASYNC).then(
    [=] (const PriceCandlestick& technicals) {
    if(technicals.get_open() != Money::ZERO) {
      m_open = technicals.get_open();
      m_openSignal(m_open);
    }
    if(technicals.get_close() != Money::ZERO) {
      m_close = technicals.get_close();
      m_closeSignal(m_close);
    }
    if(technicals.get_high() != Money::ZERO) {
      m_high = technicals.get_high();
      m_highSignal(m_high);
    }
    if(technicals.get_low() != Money::ZERO) {
      m_low = technicals.get_low();
      m_lowSignal(m_low);
    }
    if(technicals.get_volume() != 0) {
      m_volume = technicals.get_volume();
      m_volumeSignal(m_volume);
    }
    m_loadPromise = nullptr;
  }));
}

void TickerTechnicalsModel::OnTimeAndSale(const TimeAndSale& timeAndSale) {
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
