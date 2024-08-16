#include "Spire/AccountViewer/GroupProfitAndLossReportWidget.hpp"
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Spire/Blotter/ProfitAndLossWidget.hpp"
#include "Spire/UI/CollapsibleWidget.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_GroupProfitAndLossReportWidget.h"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

GroupProfitAndLossReportWidget::ReportModel::ReportModel(
    Ref<UserProfile> userProfile, ScopedQueueReader<const Order*> orders)
    : m_profitAndLossModel(Ref(userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates()), false),
      m_portfolioController(Beam::Initialize(userProfile->GetMarketDatabase()),
        &userProfile->GetServiceClients().GetMarketDataClient(),
        std::move(orders)) {
  m_profitAndLossModel.SetPortfolioController(Ref(m_portfolioController));
}

GroupProfitAndLossReportWidget::GroupProfitAndLossReportWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_GroupProfitAndLossReportWidget>()),
      m_userProfile(nullptr) {
  m_ui->setupUi(this);
  connect(m_ui->m_updateButton, &QPushButton::clicked, this,
    &GroupProfitAndLossReportWidget::OnUpdate);
}

GroupProfitAndLossReportWidget::~GroupProfitAndLossReportWidget() {}

void GroupProfitAndLossReportWidget::Initialize(
    Ref<UserProfile> userProfile, const DirectoryEntry& group) {
  m_userProfile = userProfile.Get();
  m_group = group;
  m_ui->m_fromPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
  m_ui->m_toPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
}

void GroupProfitAndLossReportWidget::OnUpdate(bool checked) {
  while(m_ui->m_traderProfitAndLossLayout->count() > 0) {
    auto item = m_ui->m_traderProfitAndLossLayout->takeAt(0);
    item->widget()->close();
    delete item;
  }
  repaint();
  m_groupModels.clear();
  auto startTime = ToUtcTime(
    ToPosixTime(m_ui->m_fromPeriodDateEdit->dateTime()));
  auto endTime = ToUtcTime(ToPosixTime(m_ui->m_toPeriodDateEdit->dateTime()));
  auto traderDirectory = m_userProfile->GetServiceClients().
    GetServiceLocatorClient().LoadDirectoryEntry(m_group, "traders");
  auto traders = m_userProfile->GetServiceClients().GetServiceLocatorClient().
    LoadChildren(traderDirectory);
  sort(traders.begin(), traders.end(),
    [] (const DirectoryEntry& lhs, const DirectoryEntry& rhs) {
      return lhs.m_name < rhs.m_name;
    });
  auto orderQueues = std::make_shared<MultiQueueWriter<const Order*>>();
  for(auto& trader : traders) {
    auto orders = std::make_shared<Queue<const Order*>>();
    QueryDailyOrderSubmissions(trader, startTime, endTime,
      m_userProfile->GetMarketDatabase(), m_userProfile->GetTimeZoneDatabase(),
      m_userProfile->GetServiceClients().GetOrderExecutionClient(), orders);
    auto reportModel = std::make_unique<ReportModel>(Ref(*m_userProfile),
      orders);
    m_groupModels.push_back(std::move(reportModel));
    QueryDailyOrderSubmissions(trader, startTime, endTime,
      m_userProfile->GetMarketDatabase(), m_userProfile->GetTimeZoneDatabase(),
      m_userProfile->GetServiceClients().GetOrderExecutionClient(),
      orderQueues->GetWriter());
  }
  m_totalsModel = std::nullopt;
  m_totalsModel.emplace(Ref(*m_userProfile), std::move(orderQueues));
  auto totalsProfitAndLossWidget = new ProfitAndLossWidget();
  totalsProfitAndLossWidget->SetModel(Ref(*m_userProfile),
    Ref(m_totalsModel->m_profitAndLossModel));
  auto totalsWidget = new CollapsibleWidget(tr("Totals"),
    totalsProfitAndLossWidget, true);
  totalsWidget->setSizePolicy(QSizePolicy(
    totalsWidget->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed));
  m_ui->m_traderProfitAndLossLayout->addWidget(totalsWidget);
  auto count = 0;
  for(auto& reportModel : m_groupModels) {
    auto profitAndLossWidget = new ProfitAndLossWidget();
    profitAndLossWidget->SetModel(Ref(*m_userProfile),
      Ref(reportModel->m_profitAndLossModel));
    auto reportWidget = new CollapsibleWidget(QString::fromStdString(
      traders[count].m_name), profitAndLossWidget, false);
    reportWidget->setSizePolicy(QSizePolicy(
      reportWidget->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed));
    m_ui->m_traderProfitAndLossLayout->addWidget(reportWidget);
    ++count;
  }
  m_ui->m_profitAndLossScrollArea->widget()->layout()->addItem(
    new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
