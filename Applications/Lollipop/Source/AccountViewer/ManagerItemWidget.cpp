#include "Spire/AccountViewer/ManagerItemWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "ui_ManagerItemWidget.h"

using namespace Beam;
using namespace Spire;
using namespace std;

ManagerItemWidget::ManagerItemWidget(Ref<UserProfile> userProfile,
    const std::shared_ptr<AccountInfoModel>& infoModel, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_ManagerItemWidget>()),
      m_infoModel(infoModel) {
  m_ui->setupUi(this);
  m_ui->m_accountInfoTab->Initialize(Ref(userProfile), false, false);
  m_ui->m_accountInfoTab->SetModel(m_infoModel);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &ManagerItemWidget::OnApply);
  connect(m_ui->m_revertButton, &QPushButton::clicked, this,
    &ManagerItemWidget::OnRevert);
}

ManagerItemWidget::~ManagerItemWidget() {}

void ManagerItemWidget::OnApply() {
  try {
    m_infoModel->Commit();
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save the account's info."));
  }
}

void ManagerItemWidget::OnRevert() {
  try {
    m_infoModel->Load();
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the account's info."));
  }
}
