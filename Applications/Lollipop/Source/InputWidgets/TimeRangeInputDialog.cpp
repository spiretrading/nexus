#include "Spire/InputWidgets/TimeRangeInputDialog.hpp"
#include "ui_TimeRangeInputDialog.h"

using namespace Beam;
using namespace Spire;

TimeRangeInputDialog::TimeRangeInputDialog(const TimeRangeParameter& startTime,
    const TimeRangeParameter& endTime, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_TimeRangeInputDialog>()) {
  m_ui->setupUi(this);
  m_ui->m_timeRangeWidget->SetTimeRange(startTime, endTime);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &TimeRangeInputDialog::accept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &TimeRangeInputDialog::reject);
}

TimeRangeInputDialog::~TimeRangeInputDialog() {}

TimeRangeParameter TimeRangeInputDialog::GetStartTime() const {
  return m_ui->m_timeRangeWidget->GetStartTime();
}

TimeRangeParameter TimeRangeInputDialog::GetEndTime() const {
  return m_ui->m_timeRangeWidget->GetEndTime();
}
