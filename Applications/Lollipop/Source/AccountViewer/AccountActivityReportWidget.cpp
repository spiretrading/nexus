#include "Spire/AccountViewer/AccountActivityReportWidget.hpp"
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_AccountActivityReportWidget.h"

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

AccountActivityReportWidget::ReportModel::ReportModel(
    Ref<UserProfile> userProfile, ScopedQueueReader<const Order*> orders)
    : m_profitAndLossModel(Ref(userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates()), false),
      m_portfolioController(Beam::Initialize(userProfile->GetMarketDatabase()),
        &userProfile->GetServiceClients().GetMarketDataClient(),
        std::move(orders)) {
  m_profitAndLossModel.SetPortfolioController(Ref(m_portfolioController));
}

AccountActivityReportWidget::AccountActivityReportWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_AccountActivityReportWidget>()),
      m_userProfile(nullptr) {
  m_ui->setupUi(this);
  connect(m_ui->m_updateButton, &QPushButton::clicked, this,
    &AccountActivityReportWidget::OnUpdate);
}

AccountActivityReportWidget::~AccountActivityReportWidget() {}

void AccountActivityReportWidget::Initialize(Ref<UserProfile> userProfile,
    const DirectoryEntry& account) {
  m_userProfile = userProfile.Get();
  m_account = account;
  m_ui->m_fromPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
  m_ui->m_toPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
}

void AccountActivityReportWidget::OnUpdate(bool checked) {
  auto startTime = ToUtcTime(
    ToPosixTime(m_ui->m_fromPeriodDateEdit->dateTime()));
  auto endTime = ToUtcTime(ToPosixTime(m_ui->m_toPeriodDateEdit->dateTime()));
  auto orders = std::make_shared<Queue<const Order*>>();
  QueryDailyOrderSubmissions(m_account, startTime, endTime,
    m_userProfile->GetMarketDatabase(), m_userProfile->GetTimeZoneDatabase(),
    m_userProfile->GetServiceClients().GetOrderExecutionClient(), orders);
  m_model.emplace(Ref(*m_userProfile), orders);
  m_ui->m_profitAndLossWidget->SetModel(Ref(*m_userProfile),
    Ref(m_model->m_profitAndLossModel));
}
