#include "Spire/RiskTimer/RiskTimerDialog.hpp"
#include <QTime>
#include "Spire/RiskTimer/RiskTimerModel.hpp"
#include "ui_RiskTimerDialog.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace std;

RiskTimerDialog::RiskTimerDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_RiskTimerDialog>()) {
  m_ui->setupUi(this);
  QPalette palette;
  palette.setColor(QPalette::WindowText,Qt::red);
  m_ui->m_remainingTimeCounter->setPalette(palette);
}

RiskTimerDialog::~RiskTimerDialog() {}

void RiskTimerDialog::SetModel(const std::shared_ptr<RiskTimerModel>& model) {
  m_model = model;
  m_timeRemainingConnection = m_model->ConnectTimeRemainingSignal(
    std::bind(&RiskTimerDialog::OnTimeRemainingUpdated, this,
    std::placeholders::_1));
  OnTimeRemainingUpdated(m_model->GetTimeRemaining());
}

void RiskTimerDialog::OnTimeRemainingUpdated(
    const time_duration& timeRemaining) {
  QTime remainingTimeDisplay(0, 0, 0, 0);
  remainingTimeDisplay = remainingTimeDisplay.addMSecs(
    static_cast<int>(timeRemaining.total_milliseconds()));
  m_ui->m_remainingTimeCounter->display(remainingTimeDisplay.toString(
    QString("hh:mm:ss")));
  if(timeRemaining <= seconds(0)) {
    m_ui->m_remainingTimeGroup->setTitle("DISABLED");
  } else {
    m_ui->m_remainingTimeGroup->setTitle("CLOSED ORDERS");
  }
}
