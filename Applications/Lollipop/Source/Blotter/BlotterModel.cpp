#include "Spire/Blotter/BlotterModel.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Spire/Blotter/CancelOnFillController.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

BlotterModel::BlotterModel(const std::string& name,
    const DirectoryEntry& executingAccount, bool isConsolidated,
    Ref<UserProfile> userProfile, const BlotterTaskProperties& taskProperties,
    const OrderLogProperties& orderLogProperties)
    : m_name(name),
      m_executingAccount(executingAccount),
      m_isConsolidated(isConsolidated),
      m_userProfile(userProfile.get()),
      m_isPersistent(false),
      m_tasksModel(Ref(*m_userProfile), executingAccount, m_isConsolidated,
        taskProperties),
      m_orderLogModel(orderLogProperties),
      m_profitAndLossModel(Ref(m_userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates()), true) {
  m_userProfile->GetClients().get_administration_client().
    get_risk_parameters_publisher(m_executingAccount).monitor(
      m_eventHandler.get_slot<RiskParameters>(
        std::bind_front(&BlotterModel::OnRiskParametersChanged, this)));
  InitializeModels();
}

BlotterModel::~BlotterModel() {
  while(!m_incomingLinks.empty()) {
    Unlink(*m_incomingLinks.back());
  }
  while(!m_outgoingLinks.empty()) {
    m_outgoingLinks.back()->Unlink(*this);
  }
}

const std::string& BlotterModel::GetName() const {
  return m_name;
}

const DirectoryEntry& BlotterModel::GetExecutingAccount() const {
  return m_executingAccount;
}

bool BlotterModel::IsConsolidated() const {
  return m_isConsolidated;
}

bool BlotterModel::IsPersistent() const {
  return m_isPersistent;
}

void BlotterModel::SetPersistent(bool value) {
  m_isPersistent = value;
}

const BlotterTasksModel& BlotterModel::GetTasksModel() const {
  return m_tasksModel;
}

BlotterTasksModel& BlotterModel::GetTasksModel() {
  return m_tasksModel;
}

const OrderLogModel& BlotterModel::GetOrderLogModel() const {
  return m_orderLogModel;
}

OrderLogModel& BlotterModel::GetOrderLogModel() {
  return m_orderLogModel;
}

const OpenPositionsModel& BlotterModel::GetOpenPositionsModel() const {
  return m_openPositionsModel;
}

OpenPositionsModel& BlotterModel::GetOpenPositionsModel() {
  return m_openPositionsModel;
}

const ProfitAndLossModel& BlotterModel::GetProfitAndLossModel() const {
  return m_profitAndLossModel;
}

ProfitAndLossModel& BlotterModel::GetProfitAndLossModel() {
  return m_profitAndLossModel;
}

const ActivityLogModel& BlotterModel::GetActivityLogModel() const {
  return m_activityLogModel;
}

ActivityLogModel& BlotterModel::GetActivityLogModel() {
  return m_activityLogModel;
}

const std::vector<BlotterModel*>& BlotterModel::GetLinkedBlotters() const {
  return m_incomingLinks;
}

void BlotterModel::Link(Ref<BlotterModel> blotter) {
  if(find(m_incomingLinks.begin(), m_incomingLinks.end(), blotter.get()) !=
      m_incomingLinks.end()) {
    return;
  }
  m_incomingLinks.push_back(blotter.get());
  blotter->m_outgoingLinks.push_back(this);
  m_tasksModel.Link(Ref(blotter->GetTasksModel()));
}

void BlotterModel::Unlink(BlotterModel& blotter) {
  auto blotterIterator = find(m_incomingLinks.begin(), m_incomingLinks.end(),
    &blotter);
  if(blotterIterator == m_incomingLinks.end()) {
    return;
  }
  m_tasksModel.Unlink(blotter.GetTasksModel());
  m_incomingLinks.erase(blotterIterator);
  blotter.m_outgoingLinks.erase(find(blotter.m_outgoingLinks.begin(),
    blotter.m_outgoingLinks.end(), this));
  auto outgoingLinks = m_outgoingLinks;
  for(auto& outgoingLink : outgoingLinks) {
    outgoingLink->Unlink(*this);
  }
  InitializeModels();
  for(auto& outgoingLink : outgoingLinks) {
    outgoingLink->Link(Ref(*this));
  }
}

void BlotterModel::InitializeModels() {
  auto& orderExecutionPublisher = m_tasksModel.GetOrderExecutionPublisher();
  if(m_isConsolidated) {
    auto [portfolio, sequence, excludedOrders] = make_portfolio(
      m_userProfile->GetClients().get_risk_client().load_inventory_snapshot(
        m_executingAccount), m_executingAccount,
      m_userProfile->GetVenueDatabase(),
      m_userProfile->GetClients().get_order_execution_client());
    auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
    for(auto& order : excludedOrders) {
      orders->push(order);
    }
    auto query = AccountQuery();
    query.set_index(m_executingAccount);
    query.set_range(increment(sequence), Beam::Sequence::LAST);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    m_userProfile->GetClients().get_order_execution_client().query(
      query, orders);
    m_portfolioController.emplace(std::move(portfolio),
      &m_userProfile->GetClients().get_market_data_client(),
      std::move(orders));
  } else {
    auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
    orderExecutionPublisher.monitor(orders);
    m_portfolioController.emplace(
      Portfolio<TrueAverageBookkeeper>(m_userProfile->GetVenueDatabase()),
      &m_userProfile->GetClients().get_market_data_client(), std::move(orders));
  }
  m_orderLogModel.SetOrderExecutionPublisher(Ref(orderExecutionPublisher));
  m_openPositionsModel.SetPortfolioController(Ref(*m_portfolioController));
  m_profitAndLossModel.SetPortfolioController(Ref(*m_portfolioController));
  m_activityLogModel.SetOrderExecutionPublisher(Ref(orderExecutionPublisher));
  auto currentAccount =
    m_userProfile->GetClients().get_service_locator_client().get_account();
  if(m_isConsolidated && m_executingAccount == currentAccount) {
    m_cancelOnFillController =
      std::make_unique<CancelOnFillController>(Ref(*m_userProfile));
    m_cancelOnFillController->SetOrderExecutionPublisher(
      Ref(orderExecutionPublisher));
  }
}

void BlotterModel::OnRiskParametersChanged(
    const RiskParameters& riskParameters) {
  m_profitAndLossModel.SetCurrency(riskParameters.m_currency);
}
