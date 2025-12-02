#include "Spire/AccountViewer/AccountActivityReportWidget.hpp"
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_AccountActivityReportWidget.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

AccountActivityReportWidget::ReportModel::ReportModel(
    Ref<UserProfile> userProfile,
    ScopedQueueReader<std::shared_ptr<Order>> orders)
    : m_profitAndLossModel(Ref(userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates()), false),
      m_portfolioController(Beam::init(userProfile->GetVenueDatabase()),
        &userProfile->GetClients().get_market_data_client(),
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
  m_userProfile = userProfile.get();
  m_account = account;
  m_ui->m_fromPeriodDateEdit->setDate(ToQDateTime(to_local_time(
    m_userProfile->GetClients().get_time_client().get_time())).date());
  m_ui->m_toPeriodDateEdit->setDate(ToQDateTime(to_local_time(
    m_userProfile->GetClients().get_time_client().get_time())).date());
}

void AccountActivityReportWidget::OnUpdate(bool checked) {
  auto startTime = to_utc_time(
    ToPosixTime(m_ui->m_fromPeriodDateEdit->dateTime()));
  auto endTime = to_utc_time(ToPosixTime(m_ui->m_toPeriodDateEdit->dateTime()));
  auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
  query_daily_order_submissions(m_account, startTime, endTime,
    m_userProfile->GetVenueDatabase(), m_userProfile->GetTimeZoneDatabase(),
    m_userProfile->GetClients().get_order_execution_client(), orders);
  m_model.emplace(Ref(*m_userProfile), orders);
  m_ui->m_profitAndLossWidget->SetModel(Ref(*m_userProfile),
    Ref(m_model->m_profitAndLossModel));
}
