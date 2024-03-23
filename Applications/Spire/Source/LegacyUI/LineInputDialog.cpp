#include "Spire/LegacyUI/LineInputDialog.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "ui_LineInputDialog.h"

using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

LineInputDialog::LineInputDialog(const string& title, const string& inputLabel,
    const string& initialInput, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_LineInputDialog>()) {
  m_ui->setupUi(this);
  setFixedSize(scale(size()));
  m_ui->m_inputLayout->setHorizontalSpacing(
    scale_width(m_ui->m_inputLayout->horizontalSpacing()));
  m_ui->m_buttonLayout->setSpacing(
    scale_width(m_ui->m_buttonLayout->spacing()));
  setWindowTitle(QString::fromStdString(title));
  m_ui->m_inputLabel->setText(QString::fromStdString(inputLabel));
  m_ui->m_input->setText(QString::fromStdString(initialInput));
  m_ui->m_input->selectAll();
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &LineInputDialog::accept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &LineInputDialog::reject);
}

LineInputDialog::~LineInputDialog() {}

string LineInputDialog::GetInput() const {
  return m_ui->m_input->text().toStdString();
}
